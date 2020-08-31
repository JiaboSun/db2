#pragma once

#include "config.h"
#include "common/global.h"

class table_t;
class Catalog;
class txn_man;
namespace dbx1000 {
    class RowItem;
    class ManagerInstance;
};

// Only a constant number of versions can be maintained.
// If a request accesses an old version that has been recycled,   
// simply abort the request.

#if CC_ALG == MVCC
struct WriteHisEntry {
	bool valid;		// whether the entry contains a valid version
	bool reserved; 	// when valid == false, whether the entry is reserved by a P_REQ 
	ts_t ts;
	dbx1000::RowItem * row;
};

struct ReqEntry {
	bool valid;
	TsType type; // P_REQ or R_REQ
	ts_t ts;        //! 当前事务开始时间
	txn_man * txn;  //! 当前事务
	ts_t time;      //! 分配出该 ReqEntry 的时间戳
	//! 两个时间戳并不一样，ts 是全局从 0 递增的，time 是 get_sys_clock()
};


class Row_mvcc {
public:
	void init(uint64_t key, size_t size, dbx1000::ManagerInstance*);
	RC access(txn_man * txn, TsType type, dbx1000::RowItem * row);
private:
 	/* pthread_mutex_t * latch; */
	volatile bool blatch;
	volatile bool recycle_latch;

	/* row_t * _row; */
	uint64_t key_;
	size_t size_;
	dbx1000::RowItem* row_;
	dbx1000::ManagerInstance* manager_instance_;

	/* RC conflict(TsType type, ts_t ts, uint64_t thd_id = 0); */
	void update_buffer(txn_man * txn, TsType type);
	void buffer_req(TsType type, txn_man * txn, bool served);

	// Invariant: all valid entries in _requests have greater ts than any entry in _write_history 
	dbx1000::RowItem * 		_latest_row;
	ts_t			_latest_wts;
	ts_t			_oldest_wts;
	WriteHisEntry * _write_history;
	// the following is a small optimization.
	// the timestamp for the served prewrite request. There should be at most one 
	// served prewrite request. 
	bool  			_exists_prewrite;
	ts_t 			_prewrite_ts;
	uint32_t 		_prewrite_his_id;
	ts_t 			_max_served_rts;

	// _requests only contains pending requests.
	ReqEntry * 		_requests;
	uint32_t 		_his_len;
	uint32_t 		_req_len;
	// Invariant: _num_versions <= 4
	// Invariant: _num_prewrite_reservation <= 2
	uint32_t 		_num_versions;
	
	// list = 0: _write_history
	// list = 1: _requests
	void double_list(uint32_t list);
	dbx1000::RowItem * reserveRow(ts_t ts, txn_man * txn);

	bool flush_req_;
//	bool
	bool Invalid();
	void GetLatestRow(txn_man * txn);
	void GetLatestRowForTest(txn_man * txn);
	void CheckLatestRow();
	bool RecycleALL();
	void PrintWriteHistory(ts_t ts);
};

#endif
