#pragma once 

#include <memory>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <vector>
#include "common/global.h"

//class row_t;
class table_t;
//class IndexHash;
//class index_btree;
class Catalog;
class lock_man;
//class txn_man;
//class thread_t;
//class index_base;
class Timestamp;
//class Mvcc;
class Row_mvcc;

namespace leveldb {
    class DB;
}
namespace dbx1000{
    class Arena;
    class Buffer;
    class MemoryDB;
    class TableSpace;
    class Index;
}

//! workload 基类，tables (表名和table指针集各)，indexes（表名对应的索引）
//! 操作包括初始化 表空间、初始化表（包括填充表格里的数据）
//! 插入行等
//! get_txn_man
// this is the base class for all workload
class workload
{
public:
	// tables indexed by table name
	map<string, table_t *> tables;
//	map<string, INDEX *> indexes;

	workload();
	virtual ~workload();
	// initialize the tables and indexes.
	virtual RC init();
	virtual RC init_schema(string schema_file);
	virtual RC init_table() = 0;
//	virtual RC get_txn_man(txn_man *& txn_manager, thread_t * h_thd)=0;

	//! 标志模拟是否完成，即事务是否执行完指定的数量。
	//! 存在的疑问：在 thread_t::run() 最后，sim_done 为所有线程共享数据，
	//! 是否在一个线程达到退出条件后，其他的线程检测到 _wl->sim_done==true，就直接退出了，且不管是否执行完？
//	bool sim_done;
	std::atomic<bool> sim_done_;
	std::vector<dbx1000::Arena*> arenas_;
	dbx1000::Buffer* buffer_;

protected:
//	void index_insert(string index_name, uint64_t key, row_t * row);
//	void index_insert(INDEX * index, uint64_t key, row_t * row, int64_t part_id = -1);
};

