//
// Created by rrzhang on 2020/8/19.
//

#ifndef DBX1000_GLOBAL_LOCK_SERVICE_H
#define DBX1000_GLOBAL_LOCK_SERVICE_H

#include <brpc/channel.h>
#include <butil/logging.h>
#include <brpc/server.h>
#include <thread>
#include "common/lock_table/lock_table.h"
#include "common/global.h"
#include "global_lock_service.pb.h"

namespace dbx1000 {
    namespace global_lock{
        class GlobalLock;
    }
    class ManagerInstance;

    namespace global_lock_service {
        class GlobalLockServiceClient {
        public:
            GlobalLockServiceClient(const std::string&);
            GlobalLockServiceClient() = delete;
            GlobalLockServiceClient(const GlobalLockServiceClient&) = delete;
            GlobalLockServiceClient &operator=(const GlobalLockServiceClient&) = delete;

        public: /// for instance

            RC LockRemote(int instance_id, uint64_t page_id, LockMode req_mode, char *page_buf, size_t count);
            void InstanceInitDone(int instance_id);
            bool GlobalLockInitDone();
            uint64_t GetNextTs();
            ManagerInstance* manager_instance() { return this->manager_instance_; }


        public: /// for global lock
            RC Invalid(uint64_t page_id, char *page_buf, size_t count);
            // getter and setter
            global_lock::GlobalLock* global_lock() { return this->global_lock_; };

        public: /// common
            int Test();

         private:
            ManagerInstance* manager_instance_;
            global_lock::GlobalLock* global_lock_;
            ::brpc::Channel channel_;
            std::unique_ptr<dbx1000::GlobalLockService_Stub> stub_;
        };

        class GlobalLockServiceImpl : dbx1000::GlobalLockService {
        public:
            GlobalLockServiceImpl() {}
            ~GlobalLockServiceImpl() {}
            void Start( const std::string &host);

        public: /// for instance
            virtual void Invalid(::google::protobuf::RpcController* controller,
                               const ::dbx1000::InvalidRequest* request,
                               ::dbx1000::InvalidReply* response,
                               ::google::protobuf::Closure* done);
            ManagerInstance *manager_instance_;

        public: /// for global lock
            virtual void LockRemote(::google::protobuf::RpcController* controller,
                               const ::dbx1000::LockRemoteRequest* request,
                               ::dbx1000::LockRemoteReply* response,
                               ::google::protobuf::Closure* done);
            virtual void InstanceInitDone(::google::protobuf::RpcController* controller,
                               const ::dbx1000::InstanceInitDoneRequest* request,
                               ::dbx1000::InstanceInitDoneReply* response,
                               ::google::protobuf::Closure* done);
            virtual void GlobalLockInitDone(::google::protobuf::RpcController* controller,
                               const ::dbx1000::GlobalLockInitDoneRequest* request,
                               ::dbx1000::GlobalLockInitDoneReply* response,
                               ::google::protobuf::Closure* done);
            virtual void GetNextTs(::google::protobuf::RpcController* controller,
                               const ::dbx1000::GetNextTsRequest* request,
                               ::dbx1000::GetNextTsReply* response,
                               ::google::protobuf::Closure* done);

            public: /// for common
            virtual void Test(::google::protobuf::RpcController* controller,
                       const ::dbx1000::TestRequest* request,
                       ::dbx1000::TestReply* response,
                       ::google::protobuf::Closure* done);

            global_lock::GlobalLock* global_lock_;
            brpc::Server server;
        };
    }
}


#endif //DBX1000_GLOBAL_LOCK_SERVICE_H
