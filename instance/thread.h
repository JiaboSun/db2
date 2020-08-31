#pragma once 

#include <cstdlib>
#include "common/global.h"
#include "manager_instance.h"

class workload;
class base_query;
namespace dbx1000{
    class ManagerInstance;
}

//! 事务执行线程，默认为 4 个线程，每个线程持有一个 txn 对象，且每个线程有独自的 Query_thd
//! 重点在 run() 函数，里面套了个无限循环 while, 每次循环都给 txn 绑定一个 query，可以使从 Query_thd 里取得新的，也可以是上次 abort 掉的 query
//! 然后事务开始执行，并返回一个结果，成功后则 txn_cnt(成功执行的事务数量) ++，否则当前绑定的query 在后面的某个时机继续被绑定到 txn 执行
//! 当 txn_cnt 达到一定数量，则退出循环，该线程退出。
class thread_t {
public:
	uint64_t _thd_id;   //! from 0 to g_thread_cnt
	workload * _wl;     //!

	uint64_t 	get_thd_id();

	uint64_t 	get_host_cid();
	void 	 	set_host_cid(uint64_t cid);

	uint64_t 	get_cur_cid();
	void 		set_cur_cid(uint64_t cid);

	void 		init(uint64_t thd_id, workload * workload);
	// the following function must be in the form void* (*)(void*)
	// to run with pthread.
	// conversion is done within the function.
	RC 			run();

    dbx1000::ManagerInstance* manager_client_;
private:
	uint64_t 	_host_cid;
	uint64_t 	_cur_cid;
	ts_t 		_curr_ts;      //! 当前时间戳
    //! 获取下一个时间戳
	ts_t 		get_next_ts();

	/* RC	 		runTest(txn_man * txn); */
	drand48_data buffer;

	// A restart buffer for aborted txns.
	//! 当事务执行失败时，几下当前事务的 query，并给这个 query 一个随机时间，来指明下次该 query 何时执行
	struct AbortBufferEntry	{
		ts_t ready_time;                //! 下次执行该 query 的时间
		base_query * query;             //! 失败的 query
	};
	AbortBufferEntry * _abort_buffer;   //! size == _abort_buffer_size
	int _abort_buffer_size;             //! 最大失败 query 数量
	int _abort_buffer_empty_slots;      //! 未使用的 AbortBufferEntry 数量
	bool _abort_buffer_enable;          //! 标记是否启用 AbortBuffer，若false，则事务失败时后，下次事务执行的仍然是上次失败的 query
};
