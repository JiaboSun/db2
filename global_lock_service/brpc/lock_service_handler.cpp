//
// Created by rrzhang on 2020/6/10.
//
#include <iostream>
#include "lock_service_handler.h"
#include "global_lock_service_helper.h"

#include "common/buffer/buffer.h"
#include "common/index/index.h"
#include "lock_service/global_lock.h"
#include "common/storage/tablespace/page.h"
#include "config.h"

namespace dbx1000 {

    ::grpc::Status LockServiceServer::LockRemote(::grpc::ServerContext* context, const ::dbx1000::LockRemoteRequest* request, ::dbx1000::LockRemoteReply* response) {

//        std::cout << "LockServiceServer::LockRemote, instance_id : " << request->instance_id() << ", page_id : " << request->page_id() << ", count : " << request->count() << std::endl;
        RC rc;
        char *page_buf = nullptr;
        size_t count = request->count();
        if(count > 0) {
            assert(MY_PAGE_SIZE == count);
            page_buf = new char [MY_PAGE_SIZE];
        }
        else {assert(0 == count);}

        rc = manager_lock_service_->LockRemote(request->instance_id(), request->page_id(), page_buf, count);

        if(RC::TIME_OUT  == rc) {
            response->set_rc(RpcRC::TIME_OUT);
        }
        if(RC::Abort  == rc) {
            response->set_rc(RpcRC::Abort);
            return ::grpc::Status::OK;
        }
        assert(RC::RCOK == rc);
        response->set_rc(RpcRC::RCOK);
        if(count > 0) {
            response->set_page_buf(page_buf, count);
        }
        response->set_count(count);

        return ::grpc::Status::OK;
    }

    /*
    ::grpc::Status LockServiceServer::UnLockRemote(::grpc::ServerContext* context, const ::dbx1000::UnLockRemoteRequest* request, ::dbx1000::UnLockRemoteReply* response) {

        if(DBx1000ServiceHelper::DeSerializeLockMode(request->req_mode()) == LockMode::S){
            assert(manager_server_->lock_table()->UnLock(request->page_id()));
            return ::grpc::Status::OK;
        }
        if(DBx1000ServiceHelper::DeSerializeLockMode(request->req_mode()) == LockMode::X) {
            dbx1000::Page* page = new dbx1000::Page(new char[MY_PAGE_SIZE]);
            dbx1000::IndexItem indexItem;
            manager_server_->index()->IndexGet(request->key(), &indexItem);
            assert(request->count() == MY_PAGE_SIZE);
            assert(0 == manager_server_->buffer()->BufferPut(indexItem.page_id_, request->page_buf().data(), request->count()));

            assert(manager_server_->lock_table()->UnLock(request->page_id()));
            return ::grpc::Status::OK;
        }
    }*/

    ::grpc::Status LockServiceServer::InstanceInitDone(::grpc::ServerContext* context, const ::dbx1000::InstanceInitDoneRequest* request
            , ::dbx1000::InstanceInitDoneReply* response) {
        manager_lock_service_->set_instance_i(request->instance_id());
        return ::grpc::Status::OK;
    }

    ::grpc::Status LockServiceServer::LockServiceInitDone(::grpc::ServerContext* context, const ::dbx1000::LockServiceInitDoneRequest* request, ::dbx1000::LockServiceInitDoneReply* response) {
        response->set_init_done(manager_lock_service_->init_done());
        return ::grpc::Status::OK;
    }

    ::grpc::Status LockServiceServer::GetNextTs(::grpc::ServerContext* context, const ::dbx1000::GetNextTsRequest* request, ::dbx1000::GetNextTsReply* response) {
        response->set_ts(manager_lock_service_->GetNextTs(-1));
        return ::grpc::Status::OK;
    }

    ::grpc::Status LockServiceServer::GetTestNum(::grpc::ServerContext* context, const ::dbx1000::GetTestNumRequest* request, ::dbx1000::GetTestNumReply* response) {
        response->set_num(manager_lock_service_->test_num_);
        return ::grpc::Status::OK;
    }

    void LockServiceServer::Start(const std::string& host) {
        grpc::ServerBuilder builder;
        builder.AddListeningPort(host, grpc::InsecureServerCredentials());
        builder.RegisterService(this);
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << "LockServiceServer listening on : " << host << std::endl;
        server->Wait();
    }









    LockServiceClient::LockServiceClient(const std::string &addr) : stub_(dbx1000::DBx1000Service::NewStub(
            grpc::CreateChannel(addr, grpc::InsecureChannelCredentials())
    )) {
        auto channel = ::grpc::CreateChannel(addr, grpc::InsecureChannelCredentials());
        stub_ = dbx1000::DBx1000Service::NewStub(channel);

        ::grpc::ClientContext context;
        dbx1000::GetTestNumRequest request;
        dbx1000::GetTestNumReply reply;

        ::grpc::Status status = stub_->GetTestNum(&context, request, &reply);
        if(!status.ok()) {
             std::cerr << "try connet to " << addr << ", request failed: " << status.error_message() << std::endl;;
        }
        assert(status.ok());

    }

    RC LockServiceClient::Invalid(uint64_t page_id, char *page_buf, size_t count){
        InvalidRequest request;
        ::grpc::ClientContext context;
        InvalidReply reply;

        request.set_page_id(page_id);
        request.set_count(count);

        ::grpc::Status status = stub_->Invalid(&context, request, &reply);
        if(!status.ok()){
             std::cerr << "Invalid page : " << page_id << status.error_message() << std::endl;;
            return RC::Abort;
        }
//        assert(status.ok());

        if(RC::TIME_OUT == DBx1000ServiceHelper::DeSerializeRC(reply.rc())) {
            return RC::TIME_OUT;
        }
        assert(RC::RCOK == DBx1000ServiceHelper::DeSerializeRC(reply.rc()));
        if(count > 0) {
            assert(MY_PAGE_SIZE == count);
            assert(reply.count() == count);
            memcpy(page_buf, reply.page_buf().data(), count);
        }
        return RC::RCOK ;
    }
}