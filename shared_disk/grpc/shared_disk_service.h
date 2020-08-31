//
// Created by rrzhang on 2020/6/12.
//

#ifndef DBX1000_SHARED_DISK_SERVICE_H
#define DBX1000_SHARED_DISK_SERVICE_H

#include <grpcpp/grpcpp.h>
#include "shared_disk_service.grpc.pb.h"

namespace dbx1000 {
    class SharedDiskServer : public SharedDiskService::Service {
    public:
        virtual ::grpc::Status Open(::grpc::ServerContext* context, const ::dbx1000::OpenRequest* request, ::dbx1000::OpenReply* response);
        virtual ::grpc::Status Write(::grpc::ServerContext* context, const ::dbx1000::WriteRequest* request, ::dbx1000::WriteReply* response);
        virtual ::grpc::Status Read(::grpc::ServerContext* context, const ::dbx1000::ReadRequest* request, ::dbx1000::ReadReply* response);
        virtual ::grpc::Status WritePage(::grpc::ServerContext* context, const ::dbx1000::WritePageRequest* request, ::dbx1000::WritePageReply* response);
        virtual ::grpc::Status ReadPage(::grpc::ServerContext* context, const ::dbx1000::ReadPageRequest* request, ::dbx1000::ReadPageReply* response);
        virtual ::grpc::Status CloseAll(::grpc::ServerContext* context, const ::dbx1000::CloseAllRequest* request, ::dbx1000::CloseAllReply* response);
        virtual ::grpc::Status Close(::grpc::ServerContext* context, const ::dbx1000::CloseRequest* request, ::dbx1000::CloseReply* response);
        void Start(const std::string& host);
    private:
    };

    class SharedDiskClient {
    public:
        SharedDiskClient(const std::string&);
        SharedDiskClient() = delete;
        SharedDiskClient(const SharedDiskClient&) = delete;
        
        int Open(const std::string& path);
        size_t Write(int fd, const void* buf, size_t size, uint64_t offset);
        size_t Read(int fd, void* buf, size_t size, uint64_t offset);
        size_t WritePage(uint64_t page_id, const void* page_buf);
        size_t ReadPage(uint64_t page_id, void* page_buf);
        int CloseAll();
        int Close(int fd);

    private:
        std::unique_ptr<dbx1000::SharedDiskService::Stub> stub_;
    };
}


#endif //DBX1000_SHARED_DISK_SERVICE_H
