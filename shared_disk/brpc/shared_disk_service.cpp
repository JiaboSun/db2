//
// Created by rrzhang on 2020/6/12.
//
#include <brpc/server.h>
#include "shared_disk_service.h"

#include "common/storage/disk/file_io.h"
#include "config.h"

namespace dbx1000 {
    void SharedDiskServer::Open(::google::protobuf::RpcController* controller,
                           const ::dbx1000::OpenRequest* request,
                           ::dbx1000::OpenReply* response,
                           ::google::protobuf::Closure* done) {
        ::brpc::ClosureGuard done_guard(done);
        ::brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        response->set_fd(FileIO::Open(request->path()));
    }

    void SharedDiskServer::Write(::google::protobuf::RpcController* controller,
                           const ::dbx1000::WriteRequest* request,
                           ::dbx1000::WriteReply* response,
                           ::google::protobuf::Closure* done) {
        ::brpc::ClosureGuard done_guard(done);
        ::brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        assert(request->buf().size() == request->size());
        response->set_size(FileIO::Write(request->fd(), request->buf().data(), request->size(), request->offset()));
    }

    void SharedDiskServer::Read(::google::protobuf::RpcController* controller,
                           const ::dbx1000::ReadRequest* request,
                           ::dbx1000::ReadReply* response,
                           ::google::protobuf::Closure* done) {
        ::brpc::ClosureGuard done_guard(done);
        ::brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        char buf[request->size()];
        size_t read_size = FileIO::Read(request->fd(), buf, request->size(), request->offset());
        assert(read_size == request->size());
        response->set_buf(buf, read_size);
        response->set_size(read_size);
    }

    void SharedDiskServer::WritePage(::google::protobuf::RpcController* controller,
                           const ::dbx1000::WritePageRequest* request,
                           ::dbx1000::WritePageReply* response,
                           ::google::protobuf::Closure* done) {
        ::brpc::ClosureGuard done_guard(done);
        ::brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        assert(request->page_buf().size() == MY_PAGE_SIZE);
        response->set_size(FileIO::WritePage(request->page_id(), request->page_buf().data()));
    }

    void SharedDiskServer::ReadPage(::google::protobuf::RpcController* controller,
                           const ::dbx1000::ReadPageRequest* request,
                           ::dbx1000::ReadPageReply* response,
                           ::google::protobuf::Closure* done) {
        ::brpc::ClosureGuard done_guard(done);
        ::brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        char buf[MY_PAGE_SIZE];
        size_t read_size = FileIO::ReadPage(request->page_id(), buf);
        assert(read_size == MY_PAGE_SIZE);
        response->set_page_buf(buf, read_size);
        response->set_size(read_size);
    }

    void SharedDiskServer::CloseAll(::google::protobuf::RpcController* controller,
                           const ::dbx1000::CloseAllRequest* request,
                           ::dbx1000::CloseAllReply* response,
                           ::google::protobuf::Closure* done) {
        ::brpc::ClosureGuard done_guard(done);
        ::brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        response->set_rc(FileIO::Close());
    }

    void SharedDiskServer::Close(::google::protobuf::RpcController* controller,
                           const ::dbx1000::CloseRequest* request,
                           ::dbx1000::CloseReply* response,
                           ::google::protobuf::Closure* done) {
        ::brpc::ClosureGuard done_guard(done);
        ::brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        response->set_rc(FileIO::Close(request->fd()));
    }

    void SharedDiskServer::Start(const std::string& host) {

        brpc::Server server;

        if (server.AddService(this, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
            LOG(ERROR) << "Fail to add service";
            assert(false);
        }

        brpc::ServerOptions options;
        if (server.Start(host.data(), &options) != 0) {
            LOG(ERROR) << "Fail to start SharedDiskServer";
            assert(false);
        }
        server.RunUntilAskedToQuit();
    }







    SharedDiskClient::SharedDiskClient(const std::string &host) {
        brpc::ChannelOptions options;
        options.use_rdma = false;
        if (channel_.Init(host.data(), &options) != 0) {
            LOG(ERROR) << "Fail to initialize channel";
            assert(false);
        }
        stub_.reset(new dbx1000::SharedDiskService_Stub(&channel_));
    }

    int SharedDiskClient::Open(const std::string &path) {
        OpenRequest request;
        OpenReply reply;
        ::brpc::Controller cntl;

        request.set_path(path);
        stub_->Open(&cntl, &request, &reply, nullptr);
        if (!cntl.Failed()) {
            return reply.fd();
        } else {
            LOG(WARNING) << cntl.ErrorText();
            assert(false);
        }
    }

    size_t SharedDiskClient::Write(int fd, const void *buf, size_t size, uint64_t offset) {
        WriteRequest request;
        WriteReply reply;
        ::brpc::Controller cntl;

        request.set_fd(fd);
        request.set_buf(buf, size);
        request.set_size(size);
        request.set_offset(offset);

        stub_->Write(&cntl, &request, &reply, nullptr);
        if (!cntl.Failed()) {
            assert(reply.size() == size);
            return reply.size();
        } else {
            LOG(WARNING) << cntl.ErrorText();
            assert(false);
        }
    }

    size_t SharedDiskClient::Read(int fd, void *buf, size_t size, uint64_t offset) {
        ReadRequest request;
        ReadReply reply;
        ::brpc::Controller cntl;

        request.set_fd(fd);
        request.set_size(size);
        request.set_offset(offset);

        stub_->Read(&cntl, &request, &reply, nullptr);
        if (!cntl.Failed()) {
            assert(size == reply.buf().size());
            assert(size == reply.size());
            memcpy(buf, reply.buf().data(), reply.buf().size());
            return reply.size();
        } else {
            LOG(WARNING) << cntl.ErrorText();
            assert(false);
        }
    }

    size_t SharedDiskClient::WritePage(uint64_t page_id, const void *page_buf) {
        WritePageRequest request;
        WritePageReply reply;
        ::brpc::Controller cntl;

        request.set_page_id(page_id);
        request.set_page_buf(page_buf, MY_PAGE_SIZE);

        stub_->WritePage(&cntl, &request, &reply, nullptr);
        if (!cntl.Failed()) {
            assert(reply.size() == MY_PAGE_SIZE);
            return reply.size();
        } else {
            LOG(WARNING) << cntl.ErrorText();
            assert(false);
        }
    }

    size_t SharedDiskClient::ReadPage(uint64_t page_id, void *page_buf) {
        ReadPageRequest request;
        ReadPageReply reply;
        ::brpc::Controller cntl;

        request.set_page_id(page_id);

        stub_->ReadPage(&cntl, &request, &reply, nullptr);
        if (!cntl.Failed()) {
            assert(MY_PAGE_SIZE == reply.page_buf().size());
            assert(MY_PAGE_SIZE == reply.size());
            memcpy(page_buf, reply.page_buf().data(), reply.page_buf().size());
            return  reply.size();
        } else {
            LOG(WARNING) << cntl.ErrorText();
            assert(false);
        }
    }

    int SharedDiskClient::CloseAll() {
        CloseAllRequest request;
        CloseAllReply reply;
        ::brpc::Controller cntl;

        stub_->CloseAll(&cntl, &request, &reply, nullptr);
        if (!cntl.Failed()) {
            return reply.rc();
        } else {
            LOG(WARNING) << cntl.ErrorText();
            assert(false);
        }
    }

    int SharedDiskClient::Close(int fd) {
        CloseRequest request;
        CloseReply reply;
        ::brpc::Controller cntl;

        stub_->Close(&cntl, &request, &reply, nullptr);
        if (!cntl.Failed()) {
            return reply.rc();
        } else {
            LOG(WARNING) << cntl.ErrorText();
            assert(false);
        }
    }
}