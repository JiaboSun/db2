//
// Created by rrzhang on 2020/6/16.
//

#include <iostream>
#include <memory>
#include <thread>
#include <vector>

#include "common/buffer/buffer.h"
#include "common/index/index.h"
#include "common/lock_table/lock_table.h"
#include "common/storage/disk/file_io.h"
#include "common/storage/tablespace/page.h"
#include "common/storage/tablespace/row_item.h"
#include "common/storage/tablespace/tablespace.h"
#include "common/storage/catalog.h"
#include "common/storage/table.h"
#include "common/workload/ycsb_wl.h"
#include "common/workload/wl.h"
#include "common/global.h"
#include "common/myhelper.h"
#include "common/mystats.h"
#include "json/json.h"
#include "instance/benchmarks/ycsb_query.h"
#include "instance/benchmarks/query.h"
#include "instance/concurrency_control/row_mvcc.h"
#include "instance/txn/ycsb_txn.h"
#include "instance/txn/txn.h"
#include "instance/manager_instance.h"
#include "instance/thread.h"
#include "global_lock_service.h"
#include "util/parse_result.h"
#include "config.h"

using namespace std;
using namespace dbx1000::global_lock_service;

extern int parser_host(int argc, char *argv[], std::map<int, std::string> &hosts_map);
extern void Test_Lock_Table();

void RunInstanceServer(GlobalLockServiceImpl *instanceServer, dbx1000::ManagerInstance *managerInstance) {
    instanceServer->Start(managerInstance->host_map()[managerInstance->instance_id()]);
}
void f(thread_t* thread) {
    thread->run();
}


int main(int argc, char *argv[]) {
//    Test_Lock_Table();

    /**/
    assert(argc >= 2);

    dbx1000::ManagerInstance *managerInstance = new dbx1000::ManagerInstance();
    int ins_id = parser_host(argc, argv, managerInstance->host_map());
    managerInstance->set_instance_id(ins_id);
    cout << "this instane id : " << managerInstance->instance_id() << ", host : " << managerInstance->host_map()[managerInstance->instance_id()] << endl << "server id : " << managerInstance->host_map()[-1] << endl;
    managerInstance->Init(SHARED_DISK_HOST);


    {   // instance 服务端
        GlobalLockServiceImpl *globalLockService = new GlobalLockServiceImpl();
        globalLockService->manager_instance_ = managerInstance;
        thread instance_service_server(RunInstanceServer, globalLockService, managerInstance);
        instance_service_server.detach();
    }
    std::this_thread::sleep_for(chrono::seconds(2));
    // 连接集中锁管理器
    managerInstance->set_global_lock_service_client(new GlobalLockServiceClient(managerInstance->host_map()[-1]));

    managerInstance->set_init_done(true);
    managerInstance->global_lock_service_client()->InstanceInitDone(managerInstance->instance_id());

    /// 等待所有 instance 初始化完成
    while(!managerInstance->global_lock_service_client()->GlobalLockInitDone()) { std::this_thread::yield();}
    cout << "instance start." <<endl;

	warmup_finish = true;
    thread_t *thread_t_s = new thread_t[g_thread_cnt]();
    std::vector<std::thread> v_thread;
    dbx1000::Profiler profiler;
    profiler.Start();
    for(int i = 0; i < g_thread_cnt; i++) {
        thread_t_s[i].init(i, managerInstance->m_workload());
        thread_t_s[i].manager_client_ = managerInstance;
        v_thread.emplace_back(f, &thread_t_s[i]);
    }
    for(int i = 0; i < g_thread_cnt; i++) {
        v_thread[i].join();
    }
    profiler.End();

    managerInstance->stats().print(managerInstance->instance_id());
    cout << "instance run time : " << profiler.Nanos() / 1000UL << " us." << endl;
    cout << "instance throughtput : " << managerInstance->stats().txn_cnt * 1000000000L / profiler.Nanos() << " tps." << endl;
    AppendRunTime(profiler.Nanos() / 1000UL, managerInstance->instance_id());
    AppendThroughtput(managerInstance->stats().txn_cnt * 1000000000L / profiler.Nanos(), managerInstance->instance_id());


    while(1) {};
//    std::this_thread::sleep_for(chrono::seconds(10));
    delete managerInstance;

    return 0;
}
