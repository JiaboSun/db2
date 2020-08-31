//
// Created by rrzhang on 2020/4/27.
//

#ifndef DBX1000_MANAGER_INSTANCE_H
#define DBX1000_MANAGER_INSTANCE_H

#include <cstdint>
#include <atomic>
#include <unordered_map>
#include "common/lock_table/lock_table.h"
#include "common/global.h"
#include "common/mystats.h"

class txn_man;

class workload;

class Query_queue;
class Row_mvcc;

namespace dbx1000 {
    namespace global_lock_service {
        class GlobalLockServiceClient;
    }
    using namespace dbx1000::global_lock_service;
    class Buffer;
    class Index;
    class LockTable;
    class Page;
    class RowItem;
    class TableSpace;
    class SharedDiskClient;
    class RowHandler;

    class ManagerInstance {
    public:
        ManagerInstance();
        ~ManagerInstance();

        void Init(const std::string &shared_disk_host);
        void InitMvccs();

        void SetTxnMan(txn_man* m_txn);
        uint64_t GetNextTs(uint64_t thread_id);
        void AddTs(uint64_t, uint64_t);
        uint64_t GetMinTs(uint64_t);

        /// getter and setter
        int instance_id()                                               { return this->instance_id_; }
        void set_instance_id(int instance_id)                           { this->instance_id_ = instance_id; }
        void set_init_done(bool init_done)                              { this->init_done_ = init_done; }
        std::map<int, std::string>& host_map()                          { return this->host_map_; }
        Stats &stats()                                                  { return this->stats_; }
//        std::unordered_map<uint64_t, Row_mvcc*> mvcc_map()              { return this->mvcc_map_; }
        std::array<Row_mvcc*, SYNTH_TABLE_SIZE> &mvcc_array()           { return this->mvcc_array_; }
        Query_queue* query_queue()                                      { return this->query_queue_; }
        workload* m_workload()                                          { return this->m_workload_; }
        RowHandler* row_handler()                                       { return this->row_handler_; }
        Buffer* buffer()                                                { return this->buffer_; }
        TableSpace* table_space()                                       { return this->table_space_; }
        Index* index()                                                  { return this->index_; }
        LockTable* &lock_table()                                        { return this->lock_table_; }
        GlobalLockServiceClient *global_lock_service_client()                 { return this->global_lock_service_client_; }
        void set_global_lock_service_client(GlobalLockServiceClient* globalLockServiceClient)   { this->global_lock_service_client_ = globalLockServiceClient; }
        SharedDiskClient * shared_disk_client()                         { return this->shared_disk_client_; }

        int test_num_;
    private:
        int instance_id_;
        std::map<int, std::string> host_map_;
        bool init_done_;

        atomic_uint64_t timestamp_;
        uint64_t*   all_ts_;
//        std::unordered_map<uint64_t, Row_mvcc*> mvcc_map_;
        std::array<Row_mvcc*, SYNTH_TABLE_SIZE> mvcc_array_;
        txn_man** txn_man_;
        dbx1000::Stats stats_;

        Query_queue* query_queue_;
        workload* m_workload_;
        RowHandler* row_handler_;
        Buffer * buffer_;
        TableSpace* table_space_;
        Index* index_;
        LockTable* lock_table_;
        global_lock_service::GlobalLockServiceClient *global_lock_service_client_;
        SharedDiskClient * shared_disk_client_;
    };
}

#endif //DBX1000_MANAGER_INSTANCE_H
