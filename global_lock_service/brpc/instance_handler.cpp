//
// Created by rrzhang on 2020/6/10.
//
#include <cstring>
#include "instance_handler.h"
#include "global_lock_service_helper.h"

#include "instance/manager_instance.h"
#include "config.h"

namespace dbx1000 {
    ::grpc::Status InstanceServer::Invalid(::grpc::ServerContext* context, const ::dbx1000::InvalidRequest* request, ::dbx1000::InvalidReply* response) {
//        cout << "other want to invalid page " << request->page_id() << ", count : " << request->count() << endl;
        size_t count = request->count();
        if(count > 0) { assert(request->count() == MY_PAGE_SIZE); }
        else { assert(0 == count); }
        char page_buf[MY_PAGE_SIZE];
        assert(RC::RCOK == manager_instance_->lock_table()->LockInvalid(request->page_id(), page_buf, count));
        response->set_page_buf(page_buf, count);
        response->set_count(count);
        response->set_rc(RpcRC::RCOK);
        return ::grpc::Status::OK;
    }


    ::grpc::Status InstanceServer::GetTestNum(::grpc::ServerContext* context, const ::dbx1000::GetTestNumRequest* request, ::dbx1000::GetTestNumReply* response) {
        response->set_num(manager_instance_->test_num_);
        return ::grpc::Status::OK;
    }

    void InstanceServer::Start(const std::string& host){
        grpc::ServerBuilder builder;
        builder.AddListeningPort(host, grpc::InsecureServerCredentials());
        builder.RegisterService(this);
        std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
        std::cout << "InstanceServer listening on : " << host << std::endl;
        server->Wait();
    }







    InstanceClient::InstanceClient(const std::string &addr) {
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

    RC InstanceClient::LockRemote(int instance_id, uint64_t page_id, LockMode req_mode, char *page_buf, size_t count) {
//        cout << "InstanceClient::LockRemote page_id : " << page_id << ", count : " << count << endl;
        dbx1000::LockRemoteRequest request;
        ::grpc::ClientContext context;
        dbx1000::LockRemoteReply reply;

        request.set_instance_id(instance_id);
        request.set_page_id(page_id);
        request.set_req_mode(DBx1000ServiceHelper::SerializeLockMode(req_mode));
        request.set_count(count);

        ::grpc::Status status = stub_->LockRemote(&context, request, &reply);
        assert(status.ok());

        if(RC::TIME_OUT == DBx1000ServiceHelper::DeSerializeRC(reply.rc())) {
            return RC::TIME_OUT;
        }
        if(RC::Abort == DBx1000ServiceHelper::DeSerializeRC(reply.rc())) {
            return RC::Abort;
        }
        assert(RC::RCOK == DBx1000ServiceHelper::DeSerializeRC(reply.rc()));
        // 要是 server 版本比当前新，返回最新的版本
        if(count > 0) {
            assert(MY_PAGE_SIZE == count);
            assert(reply.count() == count);
            memcpy(page_buf, reply.page_buf().data(), count);
        }
        return RC::RCOK ;
    }
    /*
    bool InstanceClient::UnLockRemote(int instance_id, uint64_t page_id, uint64_t page_version, uint64_t key, uint64_t key_version, char* page_buf, size_t count) {
        dbx1000::UnLockRemoteRequest request;
        ::grpc::ClientContext context;
        dbx1000::UnLockRemoteReply reply;

        request.set_instance_id(instance_id);
        request.set_page_id(page_id);
        request.set_page_version(page_version);
        request.set_key(key);
        request.set_key_version(key_version);
        if(count > 0) {
            // 当前版本可能比 server 新
            // TODO 后续考虑去掉这部分，因为太浪费带宽
            assert(count == MY_PAGE_SIZE);
            assert(nullptr != page_buf);
            request.set_page_buf(page_buf, count);
            request.set_count(count);
        }

        ::grpc::Status status = stub_->UnLockRemote(&context, request, &reply);
        assert(status.ok());

        assert(reply.rc());
        if(reply.count() > 0) {
            // 若 server 版本比当前新，可能更新当前版本
            // TODO 后续考虑去掉这部分，因为太浪费带宽
            assert(reply.count() == MY_PAGE_SIZE);
            memcpy(page_buf, reply.page_buf().data(), MY_PAGE_SIZE);
        }
        return reply.rc();
    }*/

    void InstanceClient::InstanceInitDone(int instance_id) {
        dbx1000::InstanceInitDoneRequest request;
        ::grpc::ClientContext context;
        dbx1000::InstanceInitDoneReply reply;

        request.set_instance_id(instance_id);

        ::grpc::Status status = stub_->InstanceInitDone(&context, request, &reply);
        assert(status.ok());
    }

    bool InstanceClient::LockServiceInitDone() {
        dbx1000::LockServiceInitDoneRequest request;
        ::grpc::ClientContext context;
        dbx1000::LockServiceInitDoneReply reply;

        ::grpc::Status status = stub_->LockServiceInitDone(&context, request, &reply);
        assert(status.ok());
        return reply.init_done();
    }

    uint64_t InstanceClient::GetNextTs() {
        dbx1000::GetNextTsRequest request;
        ::grpc::ClientContext context;
        dbx1000::GetNextTsReply reply;

        ::grpc::Status status = stub_->GetNextTs(&context, request, &reply);
        assert(status.ok());
        return reply.ts();
    }

//    int InstanceClient::GetTestNum(){
//        dbx1000::GetTestNumRequest request;
//        ::grpc::ClientContext context;
//        dbx1000::GetTestNumReply reply;
//
//        ::grpc::Status status = stub_->GetTestNum(&context, request, &reply);
//        assert(status.ok());
//        return reply.num();
//    }

    ManagerInstance *InstanceClient::manager_instance() { return this->manager_instance_; }
}