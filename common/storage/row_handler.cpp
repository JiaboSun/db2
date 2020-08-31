//
// Created by rrzhang on 2020/7/13.
//

#include "row_handler.h"

#include "common/buffer/buffer.h"
#include "common/index/index.h"
#include "common/lock_table/lock_table.h"
#include "common/storage/tablespace/page.h"
#include "common/storage/tablespace/row_item.h"
#include "common/storage/tablespace/tablespace.h"
#include "common/storage/catalog.h"
#include "common/storage/table.h"
#include "common/workload/ycsb_wl.h"
#include "common/myhelper.h"
#include "common/mystats.h"
#include "json/json.h"
#include "instance/benchmarks/ycsb_query.h"
#include "instance/concurrency_control/row_mvcc.h"
#include "instance/txn/ycsb_txn.h"
#include "instance/manager_instance.h"
#include "instance/thread.h"
#include "global_lock_service.h"
#include "shared_disk_service.h"
#include "config.h"

namespace dbx1000 {

    RowHandler::RowHandler(ManagerInstance *managerInstance) : manager_instance_(managerInstance){}

    /**
     * GetRow 和 ReturnRow 在原本 dbx1000 里是属于 row_t 的函数，但是采用刷盘机制，row_t 类被去掉了
     * @param key
     * @param type
     * @param txn
     * @param row
     * @return
     */
    RC RowHandler::GetRow(uint64_t key, access_t type, txn_man *txn, RowItem *&row) {
        RC rc = RC::RCOK;
        uint64_t thd_id = txn->get_thd_id();
        TsType ts_type = (type == RD) ? R_REQ : P_REQ;
//        rc = this->mvcc_map_[key]->access(txn, ts_type, row);
        rc = this->manager_instance_->mvcc_array()[key]->access(txn, ts_type, row);
        if (rc == RC::RCOK) {
            row = txn->cur_row;
        } else if (rc == RC::WAIT) {
            dbx1000::Profiler profiler;
            profiler.Start();
            while (!txn->ts_ready) { PAUSE }
            profiler.End();
            this->manager_instance_->stats().tmp_stats[thd_id]->time_wait += profiler.Nanos();
            txn->ts_ready = true;
            row = txn->cur_row;
        }
        return rc;
    }

    void RowHandler::ReturnRow(uint64_t key, access_t type, txn_man *txn, RowItem *row) {
        RC rc = RC::RCOK;
#if CC_ALG == MVCC
        if (type == XP) {
//            this->manager_instance_->mvcc_map_[key]->access(txn, XP_REQ, row);
            this->manager_instance_->mvcc_array()[key]->access(txn, XP_REQ, row);
        } else if (type == WR) {
            assert(type == WR && row != nullptr);
//            this->manager_instance_->mvcc_map_[key]->access(txn, W_REQ, row);
            this->manager_instance_->mvcc_array()[key]->access(txn, W_REQ, row);
            assert(rc == RC::RCOK);
        }
#endif
    }
/**
 * 读一行数据到 DB（这里 DB 是缓存+磁盘）
 * @param row
 * @return
 */
    bool RowHandler::SnapShotReadRow(RowItem *row){
        auto *page = new dbx1000::Page(new char[MY_PAGE_SIZE]);
        dbx1000::IndexItem indexItem;
        this->manager_instance_->index()->IndexGet(row->key_, &indexItem);
//        assert(0 == this->manager_instance_->buffer()->BufferGetWithLock(indexItem.page_id_, page->page_buf(), MY_PAGE_SIZE));
        assert(0 == this->manager_instance_->buffer()->BufferGet(indexItem.page_id_, page->page_buf(), MY_PAGE_SIZE));
        page->Deserialize();
        assert(page->page_id() == indexItem.page_id_);

//        assert(row->size_ == ((ycsb_wl *) (this->m_workload_))->the_table->get_schema()->get_tuple_size());
        memcpy(row->row_, &page->page_buf()[indexItem.page_location_], row->size_);
        {
            /// 验证 key
            uint64_t temp_key;
            memcpy(&temp_key, row->row_, sizeof(uint64_t));           /// 读 key
            assert(row->key_ == temp_key);
        }
        return true;
    }


/**
 * 读一行数据到 DB（这里 DB 是缓存+磁盘）
 * @param row
 * @return
 */
    bool RowHandler::ReadRow(RowItem *row) {
        auto *page = new dbx1000::Page(new char[MY_PAGE_SIZE]);
        dbx1000::IndexItem indexItem;
        this->manager_instance_->index()->IndexGet(row->key_, &indexItem);
        RC rc = this->manager_instance_->lock_table()->Lock(indexItem.page_id_, dbx1000::LockMode::S);
        assert(RC::RCOK == rc);
        assert(this->manager_instance_->lock_table()->lock_table_[indexItem.page_id_]->lock_mode == LockMode::O
               || this->manager_instance_->lock_table()->lock_table_[indexItem.page_id_]->lock_mode == LockMode::S);
        assert(this->manager_instance_->lock_table()->lock_table_[indexItem.page_id_]->lock_mode != LockMode::P
               && this->manager_instance_->lock_table()->lock_table_[indexItem.page_id_]->lock_mode != LockMode::X);
        assert(0 == this->manager_instance_->buffer()->BufferGet(indexItem.page_id_, page->page_buf(), MY_PAGE_SIZE));
        page->Deserialize();
        assert(page->page_id() == indexItem.page_id_);

//        assert(row->size_ == ((ycsb_wl *) (this->m_workload_))->the_table->get_schema()->get_tuple_size());
        memcpy(row->row_, &page->page_buf()[indexItem.page_location_], row->size_);
        {
            /// 验证 key
            uint64_t temp_key;
            memcpy(&temp_key, row->row_, sizeof(uint64_t));           /// 读 key
            assert(row->key_ == temp_key);
        }
        assert(RC::RCOK == this->manager_instance_->lock_table()->UnLock(indexItem.page_id_));
        return true;
    }

/**
 * 写一行数据到 DB（这里 DB 是缓存+磁盘）
 * @param row
 * @return
 */
    bool RowHandler::WriteRow(RowItem *row) {
        auto *page = new dbx1000::Page(new char[MY_PAGE_SIZE]);
        dbx1000::IndexItem indexItem;
        this->manager_instance_->index()->IndexGet(row->key_, &indexItem);
        assert(this->manager_instance_->lock_table()->lock_table_[indexItem.page_id_]->lock_mode != LockMode::O);
        RC rc = this->manager_instance_->lock_table()->Lock(indexItem.page_id_, dbx1000::LockMode::X);
        assert(RC::RCOK == rc);
        this->manager_instance_->buffer()->BufferGet(indexItem.page_id_, page->page_buf(), MY_PAGE_SIZE);
        page->Deserialize();

//        assert(row->size_ == ((ycsb_wl *) (this->m_workload_))->the_table->get_schema()->get_tuple_size());
//        {   // some check
//            uint64_t temp_key1, temp_key2;
//            memcpy(&temp_key1, &page->page_buf()[indexItem.page_location_], sizeof(uint64_t));
//            memcpy(&temp_key2, row->row_, sizeof(uint64_t));
//            assert(temp_key1 == temp_key2);
//        }
        memcpy(&page->page_buf()[indexItem.page_location_], row->row_, row->size_);
        this->manager_instance_->buffer()->BufferPut(indexItem.page_id_, page->Serialize()->page_buf(), MY_PAGE_SIZE);
//        assert(RC::RCOK == this->manager_instance_->lock_table()->UnLock(indexItem.page_id_));
rc = this->manager_instance_->lock_table()->UnLock(indexItem.page_id_);
assert(RC::RCOK == rc);
        return true;
    }
}