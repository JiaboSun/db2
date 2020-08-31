//
// Created by rrzhang on 2020/6/12.
//

#include "shared_disk_service.h"

#include "common/storage/disk/file_io.h"
#include "config.h"

namespace dbx1000 {
    ::grpc::Status SharedDiskServer::Open(::grpc::ServerContext *context, const ::dbx1000::OpenRequest *request
                                          , ::dbx1000::OpenReply *response) {
        response->set_fd(FileIO::Open(request->path()));
        return ::grpc::Status::OK;
    }

    ::grpc::Status SharedDiskServer::Write(::grpc::ServerContext *context, const ::dbx1000::WriteRequest *request
                                           , ::dbx1000::WriteReply *response) {
        assert(request->buf().size() == request->size());
        response->set_size(FileIO::Write(request->fd(), request->buf().data(), request->size(), request->offset()));
        return ::grpc::Status::OK;
    }

    ::grpc::Status SharedDiskServer::Read(::grpc::ServerContext *context, const ::dbx1000::ReadRequest *request
                                          , ::dbx1000::ReadReply *response) {
        char buf[request->size()];
        size_t read_size = FileIO::Read(request->fd(), buf, request->size(), request->offset());
        assert(read_size == request->size());
        response->set_buf(buf, read_size);
        response->set_size(read_size);
        return ::grpc::Status::OK;
    }

    ::grpc::Status SharedDiskServer::WritePage(::grpc::ServerContext *context
                                               , const ::dbx1000::WritePageRequest *request
                                               , ::dbx1000::WritePageReply *response) {
        assert(request->page_buf().size() == MY_PAGE_SIZE);
        response->set_size(FileIO::WritePage(request->page_id(), request->page_buf().data()));
        return ::grpc::Status::OK;
    }

    ::grpc::Status SharedDiskServer::ReadPage(::grpc::ServerContext *context, const ::dbx1000::ReadPageRequest *request
                                              , ::dbx1000::ReadPageReply *response) {
        char buf[MY_PAGE_SIZE];
        size_t read_size = FileIO::ReadPage(request->page_id(), buf);
        assert(read_size == MY_PAGE_SIZE);
        response->set_page_buf(buf, read_size);
        response->set_size(read_size);
        return ::grpc::Status::OK;
    }

    ::grpc::Status SharedDiskServer::CloseAll(::grpc::ServerContext *context, const ::dbx1000::CloseAllRequest *request
                                              , ::dbx1000::CloseAllReply *response) {
        response->set_rc(FileIO::Close());
        return ::grpc::Status::OK;
    }

    ::grpc::Status SharedDiskServer::Close(::grpc::ServerContext *context, const ::dbx1000::CloseRequest *request
                                           , ::dbx1000::CloseReply *response) {
        response->set_rc(FileIO::Close(request->fd()));
        return ::grpc::Status::OK;
    }

    void SharedDiskServer::Start(const std::string& host) {
        grpc::ServerBuilder builder;
        builder.AddListeningPort(host, grpc::InsecureServerCredentials());
        builder.RegisterService(this);
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << "SharedDiskServer listening on : " << host << std::endl;
        server->Wait();
    }







    SharedDiskClient::SharedDiskClient(const std::string &host) : stub_(dbx1000::SharedDiskService::NewStub(
            grpc::CreateChannel(host, grpc::InsecureChannelCredentials())
    )) {}

    int SharedDiskClient::Open(const std::string &path) {
        OpenRequest request;
        ::grpc::ClientContext context;
        OpenReply reply;

        request.set_path(path);

        ::grpc::Status status = stub_->Open(&context, request, &reply);
        assert(status.ok());
        return reply.fd();
    }

    size_t SharedDiskClient::Write(int fd, const void *buf, size_t size, uint64_t offset) {
        WriteRequest request;
        ::grpc::ClientContext context;
        WriteReply reply;

        request.set_fd(fd);
        request.set_buf(buf, size);
        request.set_size(size);
        request.set_offset(offset);

        ::grpc::Status status = stub_->Write(&context, request, &reply);
        assert(status.ok());
        assert(reply.size() == size);
        return reply.size();
    }

    size_t SharedDiskClient::Read(int fd, void *buf, size_t size, uint64_t offset) {
        ReadRequest request;
        ::grpc::ClientContext context;
        ReadReply reply;

        request.set_fd(fd);
        request.set_size(size);
        request.set_offset(offset);

        ::grpc::Status status = stub_->Read(&context, request, &reply);
        assert(status.ok());

        assert(size == reply.buf().size());
        assert(size == reply.size());
        memcpy(buf, reply.buf().data(), reply.buf().size());
        return reply.size();
    }

    size_t SharedDiskClient::WritePage(uint64_t page_id, const void *page_buf) {
        WritePageRequest request;
        ::grpc::ClientContext context;
        WritePageReply reply;

        request.set_page_id(page_id);
        request.set_page_buf(page_buf, MY_PAGE_SIZE);

        ::grpc::Status status = stub_->WritePage(&context, request, &reply);
        assert(status.ok());
        assert(reply.size() == MY_PAGE_SIZE);
        return reply.size();
    }

    size_t SharedDiskClient::ReadPage(uint64_t page_id, void *page_buf) {
        ReadPageRequest request;
        ::grpc::ClientContext context;
        ReadPageReply reply;
        request.set_page_id(page_id);

        ::grpc::Status status = stub_->ReadPage(&context, request, &reply);
        assert(status.ok());
        assert(MY_PAGE_SIZE == reply.page_buf().size());
        assert(MY_PAGE_SIZE == reply.size());
        memcpy(page_buf, reply.page_buf().data(), reply.page_buf().size());
        return  reply.size();
    }

    int SharedDiskClient::CloseAll() {
        CloseAllRequest request;
        ::grpc::ClientContext context;
        CloseAllReply reply;

        ::grpc::Status status = stub_->CloseAll(&context, request, &reply);
        assert(status.ok());
        return reply.rc();
    }

    int SharedDiskClient::Close(int fd) {
        CloseRequest request;
        ::grpc::ClientContext context;
        CloseReply reply;

        ::grpc::Status status = stub_->Close(&context, request, &reply);
        assert(status.ok());
        return reply.rc();
    }
}