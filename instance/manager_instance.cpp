//
// Created by rrzhang on 2020/4/27.
//
#include <fstream>
#include <cassert>
#include <cstdint>
#include "manager_instance.h"

#include "common/buffer/buffer.h"
#include "common/index/index.h"
#include "common/lock_table/lock_table.h"
#include "common/storage/disk/file_io.h"
#include "common/storage/tablespace/page.h"
#include "common/storage/tablespace/row_item.h"
#include "common/storage/tablespace/tablespace.h"
#include "common/storage/catalog.h"
#include "common/storage/row_handler.h"
#include "common/storage/table.h"
#include "common/workload/ycsb_wl.h"
#include "common/workload/wl.h"
#include "common/global.h"
#include "common/myhelper.h"
#include "common/mystats.h"
#include "json/json.h"
#include "instance/benchmarks/ycsb_query.h"
#include "instance/benchmarks/query.h"
#include "instance/concurrency_control/row_mvcc.h"
#include "instance/txn/ycsb_txn.h"
#include "instance/txn/txn.h"
#include "instance/thread.h"
#include "global_lock_service.h"
#include "shared_disk_service.h"
#include "config.h"


namespace dbx1000 {

    ManagerInstance::~ManagerInstance() {
        delete all_ts_;
//        for(auto &iter:mvcc_map_) { delete iter.second; }
        for(auto &iter:mvcc_array_) { delete iter; }
        delete query_queue_;
        delete m_workload_;
        delete buffer_;
        index_->Serialize();
        table_space_->Serialize();
        delete table_space_;
        delete index_;
        delete lock_table_;
//        delete buffer_manager_rpc_handler_;
    }

    ManagerInstance::ManagerInstance() { }

    // 调用之前确保 parser_host 被调用，因为 instance_id_，host_map_ 需要先初始化
    void ManagerInstance::Init(const std::string &shared_disk_host) {
        this->init_done_ = false;
        timestamp_ = ATOMIC_VAR_INIT(1);
        this->all_ts_ = new uint64_t[g_thread_cnt]();
        for (int i = 0; i < g_thread_cnt; i++) { all_ts_[i] = UINT64_MAX; }
        this->txn_man_ = new txn_man *[g_thread_cnt]();
        stats_.init();

        this->query_queue_ = new Query_queue();
        query_queue_->init();

        this->m_workload_ = new ycsb_wl();
        m_workload_->init();
        row_handler_ = new RowHandler(this);


        InitMvccs();    // mvcc_map_ 在 m_workload_ 初始化后才能初始化

        this->table_space_ = new TableSpace(((ycsb_wl *) m_workload_)->the_table->get_table_name());
        this->index_ = new Index(((ycsb_wl *) m_workload_)->the_table->get_table_name() + "_INDEX");
        table_space_->DeSerialize();
        index_->DeSerialize();


        // instance 缓存不够时，直接刷盘会把过时的数据刷到 DB，
        // 应该把缓存调大，避免缓存不够时刷旧版本
        this->shared_disk_client_ = new SharedDiskClient(shared_disk_host);
        this->buffer_ = new Buffer(FILE_SIZE, MY_PAGE_SIZE, shared_disk_client());
        this->lock_table_ = new LockTable();

        // 无冲突时，start end ,时分区的
        // uint64_t start = (table_space_->GetLastPageId() + 1) / 32 * instance_id_;
        // uint64_t end = (table_space_->GetLastPageId() + 1) / 32 * (instance_id_ + 1);
        uint64_t start = 0;
        uint64_t end = (table_space_->GetLastPageId() + 1);
        lock_table_->Init(start, end, this->instance_id_);
        lock_table_->set_manager_instance(this);
        {   /// 预热缓存
            char buf[MY_PAGE_SIZE];
            for (uint64_t page_id = start; page_id < end; page_id++) {
                buffer_->BufferGet(page_id, buf, MY_PAGE_SIZE);
            }
        }
    }

    void ManagerInstance::InitMvccs() {
        std::cout << "ManagerInstance::InitMvccs" << std::endl;
        Profiler profiler;
        profiler.Start();
        uint32_t tuple_size = ((ycsb_wl *) m_workload_)->the_table->get_schema()->tuple_size;
        for (uint64_t  key = 0; key < g_synth_table_size; key++) {
//            mvcc_map_.insert(std::pair<uint64_t, Row_mvcc *>(key, new Row_mvcc()));
//            mvcc_map_[key]->init(key, tuple_size, this);
            mvcc_array_[key] = new Row_mvcc();
            mvcc_array_[key]->init(key, tuple_size, this);
        }
        profiler.End();
        std::cout << "ManagerInstance::InitMvccs done. time : " << profiler.Millis() << " millis." << std::endl;
    }

    uint64_t ManagerInstance::GetNextTs(uint64_t thread_id) {
//        return instance_rpc_handler_->GetNextTs();
        return timestamp_.fetch_add(1);
    }

    void ManagerInstance::SetTxnMan(txn_man *m_txn) { txn_man_[m_txn->get_thd_id()] = m_txn; }

    void ManagerInstance::AddTs(uint64_t thread_id, uint64_t ts) { all_ts_[thread_id] = ts; }

    uint64_t ManagerInstance::GetMinTs(uint64_t thread_id) {
        uint64_t min_ts = UINT64_MAX;
        for (uint32_t thd_id = 0; thd_id < g_thread_cnt; thd_id++) {
            if (min_ts > all_ts_[thd_id]) {
                min_ts = all_ts_[thd_id];
            }
        }
        return min_ts;
    }
}