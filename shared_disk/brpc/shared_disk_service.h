//
// Created by rrzhang on 2020/6/12.
//

#ifndef DBX1000_SHARED_DISK_SERVICE_H
#define DBX1000_SHARED_DISK_SERVICE_H

#include <brpc/channel.h>
#include "shared_disk_service.pb.h"

namespace dbx1000 {
    class SharedDiskServer : public SharedDiskService {
    public:
        SharedDiskServer(){}
        ~SharedDiskServer(){}
        virtual void Open(::google::protobuf::RpcController* controller,
                           const ::dbx1000::OpenRequest* request,
                           ::dbx1000::OpenReply* response,
                           ::google::protobuf::Closure* done);
        virtual void Write(::google::protobuf::RpcController* controller,
                           const ::dbx1000::WriteRequest* request,
                           ::dbx1000::WriteReply* response,
                           ::google::protobuf::Closure* done);
        virtual void Read(::google::protobuf::RpcController* controller,
                           const ::dbx1000::ReadRequest* request,
                           ::dbx1000::ReadReply* response,
                           ::google::protobuf::Closure* done);
        virtual void WritePage(::google::protobuf::RpcController* controller,
                           const ::dbx1000::WritePageRequest* request,
                           ::dbx1000::WritePageReply* response,
                           ::google::protobuf::Closure* done);
        virtual void ReadPage(::google::protobuf::RpcController* controller,
                           const ::dbx1000::ReadPageRequest* request,
                           ::dbx1000::ReadPageReply* response,
                           ::google::protobuf::Closure* done);
        virtual void CloseAll(::google::protobuf::RpcController* controller,
                           const ::dbx1000::CloseAllRequest* request,
                           ::dbx1000::CloseAllReply* response,
                           ::google::protobuf::Closure* done);
        virtual void Close(::google::protobuf::RpcController* controller,
                           const ::dbx1000::CloseRequest* request,
                           ::dbx1000::CloseReply* response,
                           ::google::protobuf::Closure* done);
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
        ::brpc::Channel channel_;
        std::unique_ptr<dbx1000::SharedDiskService_Stub> stub_;
    };
}


#endif //DBX1000_SHARED_DISK_SERVICE_H
