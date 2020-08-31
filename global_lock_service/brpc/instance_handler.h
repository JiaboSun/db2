//
// Created by rrzhang on 2020/6/10.
//

#ifndef DBX1000_INSTANCE_HANDLER_H
#define DBX1000_INSTANCE_HANDLER_H

#include <grpcpp/grpcpp.h>
#include "proto/dbx1000_service.grpc.pb.h"
#include "common/lock_table/lock_table.h"
#include "common/global.h"

namespace dbx1000 {
    class ManagerInstance;

    class InstanceServer : DBx1000Service::Service {
    public:
        virtual ::grpc::Status Invalid(::grpc::ServerContext* context, const ::dbx1000::InvalidRequest* request, ::dbx1000::InvalidReply* response);
        virtual ::grpc::Status GetTestNum(::grpc::ServerContext* context, const ::dbx1000::GetTestNumRequest* request, ::dbx1000::GetTestNumReply* response);

        void Start(const std::string& host);

        ManagerInstance *manager_instance_;
    };


    class InstanceClient{
    public:
        InstanceClient(const std::string&);
        InstanceClient() = delete;
        InstanceClient(const InstanceClient&) = delete;
        InstanceClient &operator=(const InstanceClient&) = delete;

        RC LockRemote(int instance_id, uint64_t page_id, LockMode req_mode, char *page_buf, size_t count);
        void InstanceInitDone(int instance_id);
        bool LockServiceInitDone();
        uint64_t GetNextTs();
        int GetTestNum();

        ManagerInstance* manager_instance();

     private:
        std::unique_ptr<dbx1000::DBx1000Service::Stub> stub_;
        ManagerInstance* manager_instance_;
    };
}


#endif //DBX1000_INSTANCE_HANDLER_H
