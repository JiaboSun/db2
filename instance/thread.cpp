#include <chrono>
#include <unistd.h>
#include <iostream>
#include <cassert>
#include "thread.h"

#include "common/myhelper.h"
#include "instance/benchmarks/ycsb_query.h"
#include "instance/benchmarks/query.h"
#include "instance/txn/ycsb_txn.h"
#include "instance/txn/txn.h"
#include "instance/manager_instance.h"

//! thd_id = 0...3
void thread_t::init(uint64_t thd_id, workload * workload) {
	_thd_id = thd_id;
	_wl = workload;
	srand48_r((_thd_id + 1) * std::chrono::system_clock::now().time_since_epoch().count(), &buffer);
	_abort_buffer_size = ABORT_BUFFER_SIZE;                                 //! 10
	_abort_buffer = new AbortBufferEntry[_abort_buffer_size]();
	for (int i = 0; i < _abort_buffer_size; i++)
		_abort_buffer[i].query = NULL;
	_abort_buffer_empty_slots = _abort_buffer_size;                         //! 10
	_abort_buffer_enable = (g_params["abort_buffer_enable"] == "true");     //! true
}

uint64_t thread_t::get_thd_id() { return _thd_id; }
uint64_t thread_t::get_host_cid() {	return _host_cid; }
void thread_t::set_host_cid(uint64_t cid) { _host_cid = cid; }
uint64_t thread_t::get_cur_cid() { return _cur_cid; }
void thread_t::set_cur_cid(uint64_t cid) {_cur_cid = cid; }

//! 这是每个线程的主要工作，执行事务操作，里面包含了一个无限循环，除非到达种植条件，否则事务一直执行。
//! 循环之前给每个线程分配一个事务空间（在后面循环里重复使用）
//! 一个循环流程如下：
//! 1. 给这个事务附上一个 m_query，分配事务 id(thd_id, thd_id+4, thd_id+8, ...)，rc = RCOK, m_txn->abort_cnt = 0;
//! 2. 执行 query, rc = m_txn->run_txn(m_query);
//! 3.1 若 rc = ABORT，根据 _abort_buffer_enable == false，睡眠一段时间，下次仍然执行这个 m_query;
//!                        _abort_buffer_enable == true，m_query 放到 buffer 里，并设置下次的执行时间，
//! 3.2 rc = RCOK, txn_cnt

//! 3.1 后，进入下次循环，m_txn->abort_cnt = 0; 事务的 abort 计数永远置为 0 ,rc 同时也置为 RCOK。
//! 且
RC thread_t::run() {
    /*
#if !NOGRAPHITE
	_thd_id = CarbonGetTileId();
#endif
	if (warmup_finish) {
		mem_allocator.register_thread(_thd_id);
	}
	pthread_barrier_wait( &warmup_bar );
     */
	this->manager_client_->stats().init(get_thd_id());
    /*
	pthread_barrier_wait( &warmup_bar );

	set_affinity(get_thd_id());

	myrand rdm;
	rdm.init(get_thd_id());
     */
	RC rc = RC::RCOK;
	txn_man * m_txn = new ycsb_txn_man();
    m_txn->init(this, this->_wl, this->get_thd_id());
	/* rc = _wl->get_txn_man(m_txn, this);
	assert (rc == RCOK);
	glob_manager->set_txn_man(m_txn);*/
	manager_client_->SetTxnMan(m_txn);

	base_query * m_query = NULL;
	uint64_t thd_txn_id = 0;
	uint64_t txn_cnt = 0;

	while (true) {
		/* ts_t starttime = get_sys_clock(); */

	    dbx1000::Profiler profiler;
	    profiler.Start();

		//! 该 if 选出一个 m_query
		if (WORKLOAD != TEST) {
			int trial = 0;  //! 从下面的代码来看，trial 并没有用，永远为 0
			if (_abort_buffer_enable) {
				m_query = NULL;
				while (trial < 2) {
					/* ts_t curr_time = get_sys_clock(); */
					ts_t curr_time = std::chrono::system_clock::now().time_since_epoch().count();
					ts_t min_ready_time = UINT64_MAX;
					//! 有上次 abort 的 query
					if (_abort_buffer_empty_slots < _abort_buffer_size) {
					    //! 从 _abort_buffer 挑出合适的 query，但并不一定成功
						for (int i = 0; i < _abort_buffer_size; i++) {
							if (_abort_buffer[i].query != NULL && curr_time > _abort_buffer[i].ready_time) {
								m_query = _abort_buffer[i].query;
								_abort_buffer[i].query = NULL;
								_abort_buffer_empty_slots ++;
								break;
							}
							//! 当_abort_buffer 满时，提升 query 被选中的几率
							else if (_abort_buffer_empty_slots == 0
									  && _abort_buffer[i].ready_time < min_ready_time) 
								min_ready_time = _abort_buffer[i].ready_time;
						}
					}
					if (m_query == NULL && _abort_buffer_empty_slots == 0) {
						assert(trial == 0);
						M_ASSERT(min_ready_time >= curr_time, "min_ready_time=%ld, curr_time=%ld\n", min_ready_time, curr_time);
						usleep((min_ready_time - curr_time) / 1000);
					}
					else if (m_query == NULL) {
						/* m_query = query_queue->get_next_query( _thd_id ); */
						m_query = manager_client_->query_queue()->get_next_query( _thd_id );
					#if CC_ALG == WAIT_DIE
						m_txn->set_ts(get_next_ts());
					#endif
					}
					if (m_query != NULL)
						break;
				}
			}
			//! _abort_buffer_enable  == false
			else {
			    //! 上次事务执行成功则获取新的 query，否则 m_query 仍然指向上次的 query，事务仍然执行上次的查询
				if (rc == RC::RCOK)
					m_query =  manager_client_->query_queue()->get_next_query( _thd_id );
			}
		}
		/* INC_STATS(_thd_id, time_query, get_sys_clock() - starttime); */
		profiler.End();
		this->manager_client_->stats()._stats[_thd_id]->time_query += profiler.Nanos();
		profiler.Start();
		m_txn->abort_cnt = 0;
//#if CC_ALG == VLL
//		_wl->get_txn_man(m_txn, this);
//#endif
        //! 事务 id 按线程划分，假设当前线程为 1 、总线程数为4，则事务 id 为 1, 5, 9...
		m_txn->set_txn_id(get_thd_id() + thd_txn_id * g_thread_cnt);
		thd_txn_id ++;

		if ((CC_ALG == HSTORE && !HSTORE_LOCAL_TS)
				|| CC_ALG == MVCC 
				|| CC_ALG == HEKATON
				|| CC_ALG == TIMESTAMP) {
            //! 记录事务开始时间
            m_txn->set_ts(get_next_ts());
        }

		rc = RC::RCOK;
#if CC_ALG == HSTORE
		if (WORKLOAD == TEST) {
			uint64_t part_to_access[1] = {0};
			rc = part_lock_man.lock(m_txn, &part_to_access[0], 1);
		} else 
			rc = part_lock_man.lock(m_txn, m_query->part_to_access, m_query->part_num);
#elif CC_ALG == VLL
		vll_man.vllMainLoop(m_txn, m_query);
#elif CC_ALG == MVCC || CC_ALG == HEKATON
		//! 全局数组 all_ts 记录该事务的开始时间
		/* glob_manager->add_ts(get_thd_id(), m_txn->get_ts()); */
		manager_client_->AddTs(get_thd_id(), m_txn->get_ts());
#elif CC_ALG == OCC
		// In the original OCC paper, start_ts only reads the current ts without advancing it.
		// But we advance the global ts here to simplify the implementation. However, the final
		// results should be the same.
		m_txn->start_ts = get_next_ts(); 
#endif
		if (rc == RC::RCOK)
		{
		    /*
#if CC_ALG != VLL
			if (WORKLOAD == TEST)
				rc = runTest(m_txn);
			else
		     */
				rc = m_txn->run_txn(m_query);
		    /*
#endif
#if CC_ALG == HSTORE
			if (WORKLOAD == TEST) {
				uint64_t part_to_access[1] = {0};
				part_lock_man.unlock(m_txn, &part_to_access[0], 1);
			} else 
				part_lock_man.unlock(m_txn, m_query->part_to_access, m_query->part_num);
#endif
		     */
		}
		if (rc == RC::Abort) {
			uint64_t penalty = 0;
			if (ABORT_PENALTY != 0)  {  //! ABORT_PENALTY == 100000
				double r;
				drand48_r(&buffer, &r);
				penalty = r * ABORT_PENALTY;
			}
			if (!_abort_buffer_enable)
				usleep(penalty / 1000);
			else {
				assert(_abort_buffer_empty_slots > 0);
				for (int i = 0; i < _abort_buffer_size; i ++) {
					if (_abort_buffer[i].query == NULL) {
						_abort_buffer[i].query = m_query;
						_abort_buffer[i].ready_time = std::chrono::system_clock::now().time_since_epoch().count() + penalty;
						_abort_buffer_empty_slots --;
						break;
					}
				}
			}
		}
        /*
		ts_t endtime = get_sys_clock();
		uint64_t timespan = endtime - starttime;
		INC_STATS(get_thd_id(), run_time, timespan);
		INC_STATS(get_thd_id(), latency, timespan);
         */
		profiler.End();
		this->manager_client_->stats()._stats[_thd_id]->run_time += profiler.Nanos();
		this->manager_client_->stats()._stats[_thd_id]->latency += profiler.Nanos();
		//stats.add_lat(get_thd_id(), timespan);
		if (rc == RC::RCOK) {
		    /*
			INC_STATS(get_thd_id(), txn_cnt, 1);
			stats.commit(get_thd_id());
		     */
			txn_cnt ++;
		    this->manager_client_->stats()._stats[_thd_id]->txn_cnt += 1;
			this->manager_client_->stats().commit(get_thd_id());
		} else if (rc == RC::Abort) {
		    /*
		    INC_STATS(get_thd_id(), time_abort, timespan);
			INC_STATS(get_thd_id(), abort_cnt, 1);
			stats.abort(get_thd_id());
			m_txn->abort_cnt ++;
			*/
		    // TODO
		    this->manager_client_->stats()._stats[_thd_id]->time_abort += profiler.Nanos();
		    this->manager_client_->stats()._stats[_thd_id]->abort_cnt += 1;
			this->manager_client_->stats().abort(get_thd_id());
			m_txn->abort_cnt ++;
			this->manager_client_->stats().commit(get_thd_id());
		}

		/*
		//! warmup_finish == true, 在 m_wl->init() 后，该值就为 true
		if (rc == RC::FINISH)
			return rc;
		if (!warmup_finish && txn_cnt >= WARMUP / g_thread_cnt) 
		{
			stats.clear( get_thd_id() );
			return FINISH;
		}
		 */

		//! 成功执行的事务数量 txn_cnt 达到 MAX_TXN_PER_PART 时，就退出线程
		if (warmup_finish && txn_cnt >= MAX_TXN_PER_PART) {
			assert(txn_cnt == MAX_TXN_PER_PART);
			/*
	        if( !ATOM_CAS(_wl->sim_done, false, true) )
				assert( _wl->sim_done);
			 */
			delete m_txn;
            return RC::FINISH;
	    }

		/*
		//! sim_done 为所有线程共享数据，是否在一个线程达到退出条件后，其他的线程检测到 _wl->sim_done==true，就直接退出了，且不管是否执行完？
	    if (_wl->sim_done) {
   		    return FINISH;
   		}
   		*/
	}
	assert(false);
}

ts_t thread_t::get_next_ts() {
    dbx1000::Profiler profiler;
    profiler.Start();
    uint64_t ts = manager_client_->GetNextTs(_thd_id);
    profiler.End();
    manager_client_->stats()._stats[_thd_id]->time_ts_alloc += profiler.Nanos();
    return manager_client_->GetNextTs(_thd_id);
}
/*
//! 获取下一个时间戳
ts_t
thread_t::get_next_ts() {
	if (g_ts_batch_alloc) {     //! false
		if (_curr_ts % g_ts_batch_num == 0) {   //! g_ts_batch_num = 1
			_curr_ts = glob_manager->get_ts(get_thd_id());
			_curr_ts ++;
		} else {
			_curr_ts ++;
		}
		return _curr_ts - 1;
	} else {
		_curr_ts = glob_manager->get_ts(get_thd_id());
		return _curr_ts;
	}
}

RC thread_t::runTest(txn_man * txn)
{
	RC rc = RCOK;
	if (g_test_case == READ_WRITE) {
		rc = ((TestTxnMan *)txn)->run_txn(g_test_case, 0);
#if CC_ALG == OCC
		txn->start_ts = get_next_ts(); 
#endif
		rc = ((TestTxnMan *)txn)->run_txn(g_test_case, 1);
		printf("READ_WRITE TEST PASSED\n");
		return FINISH;
	}
	else if (g_test_case == CONFLICT) {
		rc = ((TestTxnMan *)txn)->run_txn(g_test_case, 0);
		if (rc == RCOK)
			return FINISH;
		else 
			return rc;
	}
	assert(false);
	return RCOK;
}
*/