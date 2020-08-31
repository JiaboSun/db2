/*
#pragma once

#include "helper.h"
#include "global.h"

class row_t;
class txn_man;

class Manager {
public:
	void 			init();
	// returns the next timestamp.
	//! 获取时间戳
	ts_t			get_ts(uint64_t thread_id);     //! thread_id 只有在加锁方式为 TS_CLOCK 下才用的到

	// For MVCC. To calculate the min active ts in the system
	void 			add_ts(uint64_t thd_id, ts_t ts);
	ts_t 			get_min_ts(uint64_t tid = 0);

	// HACK! the following mutexes are used to model a centralized
	// lock/timestamp manager.
	//! 简单粗暴，一共有 BUCKET_CNT==31 把锁，先将 row_t * 地址一某种形式 hash 到 31 个桶内的一个，然后对该桶上锁
 	void 			lock_row(row_t * row);
	void 			release_row(row_t * row);

	txn_man * 		get_txn_man(int thd_id) { return _all_txns[thd_id]; };
	void 			set_txn_man(txn_man * txn);

	uint64_t 		get_epoch() { return *_epoch; };
	void 	 		update_epoch();
private:
	// for SILO
	volatile uint64_t * _epoch;
	ts_t * 			_last_epoch_update_time;

	pthread_mutex_t ts_mutex;
	uint64_t *		timestamp;
	pthread_mutex_t mutexes[BUCKET_CNT];
	uint64_t 		hash(row_t * row);
	ts_t volatile * volatile * volatile all_ts;
	txn_man ** 		_all_txns;
	// for MVCC
	volatile ts_t	_last_min_ts_time;
	ts_t			_min_ts;
};
*/