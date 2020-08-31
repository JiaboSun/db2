//
// Created by rrzhang on 2020/4/9.
//

#ifndef DBX1000_YCSB_WL_H
#define DBX1000_YCSB_WL_H

#include <atomic>
#include "wl.h"

class ycsb_wl : public workload {
public :
    ycsb_wl();
    virtual ~ycsb_wl();
	RC init();
	RC init_table();
	RC init_schema(string schema_file);
//	RC get_txn_man(txn_man *& txn_manager, thread_t * h_thd);
    //! 数据是分区的，每个区间的 key 都是 0-g_synth_table_size / g_part_cnt，单调增，返回 key 在哪个区间
	int key_to_part(uint64_t key);
//	INDEX * the_index;
	table_t * the_table;
private:
	void init_table_parallel();
    //! 初始化单个区间
	void * init_table_slice();
	static void * threadInitTable(void * This) {
		((ycsb_wl *)This)->init_table_slice();
		return NULL;
	}
//	pthread_mutex_t insert_lock;
//  For parallel initialization
//	static int next_tid;
	static std::atomic<int> next_tid;
};

#endif //DBX1000_YCSB_WL_H
