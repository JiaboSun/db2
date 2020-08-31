#ifndef _SYNTH_BM_H_
#define _SYNTH_BM_H_

#include "txn.h"

#include "common/lock_table/lock_table.h"
class ycsb_wl;

class ycsb_query;

/*
class ycsb_wl : public workload {
public :
	RC init();
	RC init_table();
	RC init_schema(string schema_file);
	RC get_txn_man(txn_man *& txn_manager, thread_t * h_thd);
    //! 数据是分区的，每个区间的 key 都是 0-g_synth_table_size / g_part_cnt，单调增，返回 key 在哪个区间
	int key_to_part(uint64_t key);
	INDEX * the_index;
	table_t * the_table;
private:
	void init_table_parallel();
    //! 初始化单个区间
	void * init_table_slice();
	static void * threadInitTable(void * This) {
		((ycsb_wl *)This)->init_table_slice(); 
		return NULL;
	}
	pthread_mutex_t insert_lock;
	//  For parallel initialization
	static int next_tid;
};
*/
class ycsb_txn_man : public txn_man
{
public:
	void init(thread_t * h_thd, workload * h_wl, uint64_t part_id); 
	RC run_txn(base_query * query);

	std::map<uint64_t ,dbx1000::LockMode> txn_lock_table_;
private:
	uint64_t row_cnt;
	ycsb_wl * _wl;
};

#endif
