//
// Created by rrzhang on 2020/6/16.
//
#include <iostream>
#include <thread>
#include "global_lock/global_lock.h"
#include "global_lock_service.h"
#include "config.h"
using namespace std;

extern int parser_host(int argc, char *argv[], std::map<int, std::string> &hosts_map);

void RunLockServiceServer(dbx1000::global_lock_service::GlobalLockServiceImpl * globalLockService, dbx1000::global_lock::GlobalLock* globalLock) {
    std::string port = globalLock->hosts_map()[globalLock->lock_service_id()].substr(globalLock->hosts_map()[-1].find(':'));
    globalLockService->Start("0.0.0.0" + port);
}

int main(int argc, char* argv[]){
    dbx1000::global_lock::GlobalLock* globalLock = new dbx1000::global_lock::GlobalLock();
    parser_host(argc, argv, globalLock->hosts_map());
    globalLock->set_lock_service_id(-1);

    dbx1000::global_lock_service::GlobalLockServiceImpl * globalLockService = new dbx1000::global_lock_service::GlobalLockServiceImpl();
    globalLockService->global_lock_ = globalLock;

    thread lock_service_server(RunLockServiceServer, globalLockService, globalLock);
    lock_service_server.detach();

    cout << "something." << endl;


    while(1){};
    return 0;
}