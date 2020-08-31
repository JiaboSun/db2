//
// Created by rrzhang on 2020/6/10.
//

#ifndef DBX1000_LOCK_SERVICE_HANDLER_H
#define DBX1000_LOCK_SERVICE_HANDLER_H

#include <grpcpp/grpcpp.h>
#include "dbx1000_service.grpc.pb.h"
#include "common/lock_table/lock_table.h"
#include "common/global.h"
#include "config.h"

namespace dbx1000 {
    class ManagerLockService;

    class LockServiceServer : DBx1000Service::Service {
    public:
        virtual ::grpc::Status LockRemote(::grpc::ServerContext* context, const ::dbx1000::LockRemoteRequest* request, ::dbx1000::LockRemoteReply* response);
//        virtual ::grpc::Status UnLockRemote(::grpc::ServerContext* context, const ::dbx1000::UnLockRemoteRequest* request, ::dbx1000::UnLockRemoteReply* response);
        virtual ::grpc::Status InstanceInitDone(::grpc::ServerContext* context, const ::dbx1000::InstanceInitDoneRequest* request, ::dbx1000::InstanceInitDoneReply* response);
        virtual ::grpc::Status LockServiceInitDone(::grpc::ServerContext* context, const ::dbx1000::LockServiceInitDoneRequest* request, ::dbx1000::LockServiceInitDoneReply* response);
        virtual ::grpc::Status GetNextTs(::grpc::ServerContext* context, const ::dbx1000::GetNextTsRequest* request, ::dbx1000::GetNextTsReply* response);
        virtual ::grpc::Status GetTestNum(::grpc::ServerContext* context, const ::dbx1000::GetTestNumRequest* request, ::dbx1000::GetTestNumReply* response);
        void Start(const std::string& host);

        ManagerLockService* manager_lock_service_;
    };

    class ManagerLockService;
    class LockServiceClient {
    public:
        LockServiceClient(const std::string&);
        LockServiceClient() = delete;
        LockServiceClient(const LockServiceClient&) = delete;
        LockServiceClient &operator=(const LockServiceClient&) = delete;

        RC Invalid(uint64_t page_id, char *page_buf, size_t count);

        // getter and setter
        ManagerLockService* manager_lock_service() { return this->manager_lock_service_; };
     private:
        std::unique_ptr<dbx1000::DBx1000Service::Stub> stub_;
        ManagerLockService* manager_lock_service_;
    };
}


#endif //DBX1000_LOCK_SERVICE_HANDLER_H
