#include <cstring>
#include <set>
#include <vector>
#include <thread>
#include "ycsb_txn.h"

#include "common/buffer/buffer.h"
#include "common/index/index.h"
#include "common/storage/tablespace/row_item.h"
#include "common/storage/catalog.h"
#include "common/storage/table.h"
#include "common/workload/ycsb_wl.h"
#include "common/myhelper.h"
#include "instance/benchmarks/ycsb_query.h"
#include "instance/thread.h"
#include "shared_disk_service.h"
#include "global_lock_service.h"

void ycsb_txn_man::init(thread_t * h_thd, workload * h_wl, uint64_t thd_id) {
	txn_man::init(h_thd, h_wl, thd_id);
	_wl = (ycsb_wl *) h_wl;
}

/**
 * zhangrongrong, 2020/6/27
 * for test
 * 该函数生成一个 query，共四个请求，所有请求的 key=1,2,3,4， type=WR，认为制造冲突，方便调试
 */
ycsb_query* gen_test_query(){
    ycsb_query * m_query = new ycsb_query();
    m_query->requests = new ycsb_request[g_req_per_query]();
    m_query->request_cnt = 4;
    for(int i = 0; i< 4; i++) {
        m_query->requests[i].key = i*203;
        m_query->requests[i].rtype = WR;
    }
    return m_query;
}

/**
 * zhangrongrong, 2020/6/27
 * 对每个 ycsb_query，返回里面涉及到的写 page 集合
 */
std::set<uint64_t> GetQueryWritePageSet(ycsb_query * m_query, ycsb_txn_man *ycsb){
	std::set<uint64_t> write_page_set;
    for (uint32_t rid = 0; rid < m_query->request_cnt; rid ++) {
		ycsb_request * req = &m_query->requests[rid];
        dbx1000::IndexItem indexItem;
#ifdef DB2_WITH_NO_CONFLICT
        ycsb->h_thd->manager_client_->index()->IndexGet(req->key + g_synth_table_size / MAX_PROCESS_CNT * ycsb->h_thd->manager_client_->instance_id(), &indexItem);
#else
        ycsb->h_thd->manager_client_->index()->IndexGet(req->key, &indexItem);
#endif
        if(req->rtype == WR){
	        ycsb->h_thd->manager_client_->stats()._stats[ycsb->get_thd_id()]->count_write_request_++;
            write_page_set.insert(indexItem.page_id_);
//            cout << "GetQueryWritePageSet:" << indexItem.page_id_ << endl;
        }
    }

    /// 调试用
    auto print_set = [&](){
        cout << "instance " << ycsb->h_thd->manager_client_->instance_id() << ", page_id : ";
        for(auto iter : write_page_set) {
           cout << iter << " ";
        }cout << endl;
    };
//    print_set();

    return write_page_set;
}

/**
 * zhangrongrong, 2020/6/27
 * 先检查是否有其他 instance 需要对 write_page_set 内的 page 进行 Invalid, 有的话则等
 * 然后检查本地是否有 write_page_set 所有页的写锁，没有则 RemoteLock 获取，这个函数确保当前事务在后续过程中，对涉及到的页是有写权限的
 * 然后把当前线程 id 记录到 page 内，目的是为了阻塞事务开始后，其他实例的 invalid 请求
 */
RC GetWritePageLock(std::set<uint64_t> write_page_set, ycsb_txn_man *ycsb){
    dbx1000::LockTable* lockTable =  ycsb->h_thd->manager_client_->lock_table();

    auto has_invalid_req = [&](){
        for(auto iter : write_page_set) {
            if(ycsb->h_thd->manager_client_->lock_table()->lock_table_[iter]->invalid_req){ return true; }
        }
        return false;
    };
    while(has_invalid_req()) { std::this_thread::yield(); }

//    cout << "instance " << ycsb->h_thd->manager_client_->instance_id() << ", thread " << ycsb->get_thd_id() << ", txn " << ycsb->txn_id << " get lock." << endl;

    dbx1000::Profiler profiler;
    profiler.Start();
//std::vector<thread> lock_remote_thread;
    for(auto iter : write_page_set) {
        if (iter == 0) { cout << "page 0: lock mode: " << lockTable->LockModeToChar(lockTable->lock_table_[iter]->lock_mode) << endl;}
        if(lockTable->lock_table_[iter]->lock_mode == dbx1000::LockMode::O) {
	        ycsb->h_thd->manager_client_->stats()._stats[ycsb->get_thd_id()]->count_remote_lock_++;
            bool flag = ATOM_CAS(lockTable->lock_table_[iter]->lock_remoting, false, true);
            if(flag) {
                assert(true == lockTable->lock_table_[iter]->lock_remoting);
//lock_remote_thread.emplace_back(thread([iter, lockTable, this]() {
                    char page_buf[MY_PAGE_SIZE];
//                    cout << "instance " << ycsb->h_thd->manager_client_->instance_id() << ", thread " << ycsb->get_thd_id() << ", txn " << ycsb->txn_id << " remote get lock." << endl;
#ifdef DB2_WITH_NO_CONFLICT
                    RC rc = ycsb->h_thd->manager_client_->global_lock_service_client()->LockRemote(ycsb->h_thd->manager_client_->instance_id(), iter, dbx1000::LockMode::X, nullptr, 0);
#else
                    RC rc = ycsb->h_thd->manager_client_->global_lock_service_client()->LockRemote(ycsb->h_thd->manager_client_->instance_id(), iter, dbx1000::LockMode::X, page_buf, MY_PAGE_SIZE);
#endif
                    if(RC::Abort == rc || RC::TIME_OUT == rc) {
                        lockTable->lock_table_[iter]->lock_remoting = false;
                        return RC::Abort;
                    }
                    assert(rc == RC::RCOK);
                    assert(lockTable->lock_table_[iter]->lock_mode == dbx1000::LockMode::O);
                    lockTable->lock_table_[iter]->lock_mode = dbx1000::LockMode::P;
//                    cout << "thread : " << ycsb->get_thd_id() << " change lock : " << iter << " to P" << endl;
#ifdef DB2_WITH_NO_CONFLICT
#else
                    ycsb->h_thd->manager_client_->buffer()->BufferPut(iter, page_buf, MY_PAGE_SIZE);
#endif
                    lockTable->lock_table_[iter]->lock_remoting = false;
//}
//));
            } else {
                assert(true == lockTable->lock_table_[iter]->lock_remoting);
                while(lockTable->lock_table_[iter]->lock_mode == dbx1000::LockMode::O) {
//                    cout << "thread waiting..." << endl; std::this_thread::yield();
                    }
                assert(lockTable->lock_table_[iter]->lock_mode != dbx1000::LockMode::O);
            }
        }
    }

//for(auto &iter : lock_remote_thread){
//  iter.join();
//}
    /// 把该线程请求加入 page 锁, 阻塞事务开始后，其他实例的 invalid 请求
    for(auto iter : write_page_set) {
        assert(lockTable->lock_table_[iter]->lock_mode != dbx1000::LockMode::O);
        lockTable->AddThread(iter, ycsb->get_thd_id());
    }
    profiler.End();
	ycsb->h_thd->manager_client_->stats().tmp_stats[ycsb->get_thd_id()]->time_remote_lock_ += profiler.Nanos();

    return RC::RCOK;
}


RC ycsb_txn_man::run_txn(base_query * query) {
//    cout << "txn id : " << txn_id << endl;
//    cout << "instance " << h_thd->manager_client_->instance_id() << ", thread " << this->get_thd_id() << ", txn " << this->txn_id << " start." << endl;
	RC rc;
	ycsb_query * m_query = (ycsb_query *) query;
//	ycsb_query * m_query = gen_query();         /// 调试用

    /// 事务开始前，记录下写 page 集合，同时确保这些写page的锁在本地（若不在本地，则通过 RemoteLock 获取）
    /// ,然后把线程 id 记录到 page 内，目的是为了阻塞事务开始后，其他实例的 invalid 请求
    std::set<uint64_t> write_page_set = GetQueryWritePageSet(m_query, this);
    rc = GetWritePageLock(write_page_set, this);
	if(rc == RC::Abort) { return rc; }
	/* ycsb_wl * wl = (ycsb_wl *) h_wl;
	itemid_t * m_item = NULL; */
  	row_cnt = 0;


	for (uint32_t rid = 0; rid < m_query->request_cnt; rid ++) {
	    h_thd->manager_client_->stats()._stats[get_thd_id()]->count_total_request_++;
		ycsb_request * req = &m_query->requests[rid];
		/* int part_id = wl->key_to_part( req->key ); */      //! 分区数为 1，part_id == 0
		//! finish_req、iteration 是为 req->rtype==SCAN 准备的，扫描需要读 SCAN_LEN 个 item，
		//! while 虽然为 SCAN 提供了 SCAN_LEN 次读，但是每次请求的 key 是一样的，并没有对操作 [key, key + SCAN_LEN]
		bool finish_req = false;
		uint32_t iteration = 0;
		while ( !finish_req ) {
		    /*
			if (iteration == 0) {
				m_item = index_read(_wl->the_index, req->key, part_id);
			} 
#if INDEX_STRUCT == IDX_BTREE
			else {
				_wl->the_index->index_next(get_thd_id(), m_item);
				if (m_item == NULL)
					break;
			}
#endif
			row_t * row = ((row_t *)m_item->location);
		     */
			dbx1000::RowItem * row_local;
			access_t type = req->rtype;

		/**
		 * 为了实现多个进程访问的数据没有重叠，没有冲突时，每个进程只访问 key:[g_synth_table_size/64*this->process_id, g_synth_table_size/MAX_PROCESS_CNT*(this->process_id+1)] 范围内的数据
		 * 但是，query 模块没有对应进程的 id, 所以产生的 key 范围只在 [0, g_synth_table_size/64]
		 * 在事务执行时，应该加上 : g_synth_table_size / MAX_PROCESS_CNT * process_id;
		 *
		 * 为什么是 MAX_PROCESS_CNT，而不是 PROCESS_CNT？
		 * PROCESS_CNT 下，随着 PROCESS_CNT 增大， key 范围缩小，会导致每个节点内事务同时访问的 page_id  的概率增大
		 * MAX_PROCESS_CNT 是预估最大达到的实例数
		 */
#ifdef DB2_WITH_NO_CONFLICT
            assert(h_thd->manager_client_->instance_id() < MAX_PROCESS_CNT);
            assert(req->key < g_synth_table_size / MAX_PROCESS_CNT);
            uint64_t real_key = req->key + g_synth_table_size / MAX_PROCESS_CNT * h_thd->manager_client_->instance_id();
            assert(real_key < g_synth_table_size);
			row_local = get_row(real_key , type);
#else
			row_local = get_row(req->key , type);
//			cout << "key: " << req->key << ", type:" << (req->rtype == access_t::WR ? "WR" : "RD") << endl;
#endif
			if (row_local == NULL) {
				rc = RC::Abort;
				goto final;
			}
			size_t size = _wl->the_table->get_schema()->tuple_size;
			assert(row_local->size_ == size);

			// Computation //
			// Only do computation when there are more than 1 requests.
            if (m_query->request_cnt > 1) {
                if (req->rtype == RD || req->rtype == SCAN) {
//                  for (int fid = 0; fid < schema->get_field_cnt(); fid++) {
						int fid = 0;
						char * data = row_local->row_;
						__attribute__((unused)) uint64_t fval = *(uint64_t *)(&data[fid * 10]);
//                  }
                } else {
                    assert(req->rtype == WR);
//					for (int fid = 0; fid < schema->get_field_cnt(); fid++) {
						int fid = 0;
						char * data = row_local->row_;
						/* *(uint64_t *)(&data[fid * 10]) = 0; */
						/// 写入事务 id, 当做版本号
                        memcpy(&data[size - sizeof(uint64_t)], &timestamp, sizeof(uint64_t));
//					}
                } 
            }


			iteration ++;
			if (req->rtype == RD || req->rtype == WR || iteration == req->scan_len)
				finish_req = true;
		}
	}
	rc = RC::RCOK;

final:
	rc = finish(rc);

    /// 线程结束后，把对应 page 锁内的相关信息清除，通知 invalid 函数可以执行
    dbx1000::LockTable* lockTable =  this->h_thd->manager_client_->lock_table();
    for(auto iter : write_page_set) { lockTable->RemoveThread(iter, this->get_thd_id()); }
//    cout << "instance " << h_thd->manager_client_->instance_id() << ", thread " << this->get_thd_id() << ", txn " << this->txn_id << " end." << endl;

return rc;
}