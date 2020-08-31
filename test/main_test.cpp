//
// Created by rrzhang on 2020/5/5.
//
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include "leveldb/db.h"

//#include "ycsb_query.h"
#include "server/buffer/buffer.h"
#include "server/workload/ycsb_wl.h"
#include "server/storage/table.h"
#include "server/storage/catalog.h"

#include "util/numbercomparator.h"
//#include "cs_api.h"
#include "common/global.h"
#include "server/manager_server.bak.h"
#include "client/manager_client.h"
#include "client/benchmarks/query.h"
#include "client/benchmarks/ycsb_query.h"
#include "client/thread.h"
#include "common/myhelper.h"

#include "api/api_cc/api_cc.h"
#include "api/api_txn/api_txn.h"
using namespace std;
void parser(int argc, char * argv[]);


void f(thread_t* thread) {
    thread->run();
}

int main(int argc, char* argv[]) {
	parser(argc, argv);
    stats.init();

    /// for server
    glob_manager_server = new dbx1000::ManagerServer();
    workload* m_wl;
    dbx1000::Buffer* buffer;

    m_wl = new ycsb_wl();
#ifdef USE_MEMORY_DB
    dbx1000::MemoryDB* db = new dbx1000::MemoryDB();
#else
    leveldb::DB* db;
    leveldb::Options options;
    options.create_if_missing = true;
    options.comparator = new dbx1000::NumberComparatorImpl();
    leveldb::Status status = leveldb::DB::Open(options, g_db_path, &db);
    assert(status.ok());
#endif
    m_wl->init();
    size_t tuple_size = ((ycsb_wl*)m_wl)->the_table->get_schema()->get_tuple_size();
    /// init buffer here, because 'the_table' can be use util schema be inititaled
    buffer = new dbx1000::Buffer(g_synth_table_size / 8 * tuple_size, tuple_size, db);
    m_wl->buffer_ = buffer;
    ((ycsb_wl*)m_wl)->init_table();
    cout << endl;
    glob_manager_server->InitWl((ycsb_wl*)m_wl);

    /// for txn thread
    glob_manager_client = new dbx1000::ManagerClient();
    glob_manager_client->init();
    glob_manager_client->row_size_ = tuple_size;

    query_queue = new Query_queue();
    query_queue->init();

	warmup_finish = true;

	thread_t *thread_t_s = new thread_t[g_thread_cnt]();
    std::vector<std::thread> v_thread;
    for(int i = 0; i < g_thread_cnt; i++) {
        thread_t_s[i].init(i);
        v_thread.emplace_back(f, &thread_t_s[i]);
    }

    for(int i = 0; i < g_thread_cnt; i++) {
        v_thread[i].join();
    }

    stats.print();

    cout << "exit main." << endl;
}