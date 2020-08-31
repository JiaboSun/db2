//
// Created by rrzhang on 2020/6/10.
//

#ifndef DBX1000_DBX1000_SERVICE_HELPER_H
#define DBX1000_DBX1000_SERVICE_HELPER_H

//#include "proto/dbx1000_service.grpc.pb.h"
#include "common/lock_table/lock_table.h"
#include "common/global.h"
#include "global_lock_service.pb.h"

namespace dbx1000 {
    namespace global_lock_service {
        class GlobalLockServiceHelper {
        public:
            static RpcLockMode SerializeLockMode(LockMode);

            static LockMode DeSerializeLockMode(RpcLockMode);

            static RpcRC SerializeRC(RC rc);

            static RC DeSerializeRC(RpcRC rpcRc);
        };
    }
}


#endif //DBX1000_DBX1000_SERVICE_HELPER_H
