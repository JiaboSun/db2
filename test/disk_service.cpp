//
// Created by rrzhang on 2020/6/12.
//

#include <iostream>
#include "shared_disk_service.h"
#include "config.h"
using namespace std;

int main() {

    dbx1000::SharedDiskServer* server = new dbx1000::SharedDiskServer();
    server->Start(SHARED_DISK_HOST);

    return 0;
}