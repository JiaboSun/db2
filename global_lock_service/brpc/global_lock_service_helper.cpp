//
// Created by rrzhang on 2020/6/10.
//

#include "global_lock_service_helper.h"

namespace dbx1000 {


    namespace global_lock_service {
        RpcLockMode GlobalLockServiceHelper::SerializeLockMode(LockMode mode) {
            if (LockMode::O == mode) { return RpcLockMode::O; }
            if (LockMode::S == mode) { return RpcLockMode::S; }
            if (LockMode::P == mode) { return RpcLockMode::P; }
            if (LockMode::X == mode) { return RpcLockMode::X; }
        }

        RpcRC GlobalLockServiceHelper::SerializeRC(RC rc) {
            if (RC::RCOK == rc) { return RpcRC::RCOK; }
            if (RC::Commit == rc) { return RpcRC::Commit; }
            if (RC::Abort == rc) { return RpcRC::Abort; }
            if (RC::WAIT == rc) { return RpcRC::WAIT; }
            if (RC::ERROR == rc) { return RpcRC::ERROR; }
            if (RC::FINISH == rc) { return RpcRC::FINISH; }
            if (RC::TIME_OUT == rc) { return RpcRC::TIME_OUT; }
        }

        LockMode GlobalLockServiceHelper::DeSerializeLockMode(RpcLockMode mode) {
            if (RpcLockMode::O == mode) { return LockMode::O; }
            if (RpcLockMode::S == mode) { return LockMode::S; }
            if (RpcLockMode::P == mode) { return LockMode::P; }
            if (RpcLockMode::X == mode) { return LockMode::X; }
        }

        RC GlobalLockServiceHelper::DeSerializeRC(RpcRC rpcRc) {
            if (RpcRC::RCOK == rpcRc) { return RC::RCOK; }
            if (RpcRC::Commit == rpcRc) { return RC::Commit; }
            if (RpcRC::Abort == rpcRc) { return RC::Abort; }
            if (RpcRC::WAIT == rpcRc) { return RC::WAIT; }
            if (RpcRC::ERROR == rpcRc) { return RC::ERROR; }
            if (RpcRC::FINISH == rpcRc) { return RC::FINISH; }
            if (RpcRC::TIME_OUT == rpcRc) { return RC::TIME_OUT; }
        }
    }
}