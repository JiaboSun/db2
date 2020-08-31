//
// Created by rrzhang on 2020/8/19.
//

#include <brpc/channel.h>
#include <butil/time.h>
#include <brpc/log.h>
#include <cassert>
#include "global_lock_service.h"

#include "global_lock_service_helper.h"
#include "global_lock/global_lock.h"
#include "common/lock_table/lock_table.h"
#include "instance/manager_instance.h"

namespace dbx1000 {
    namespace global_lock_service {
        GlobalLockServiceClient::GlobalLockServiceClient(const std::string &addr) {
            brpc::ChannelOptions options;
            options.use_rdma = false;
            if (channel_.Init(addr.data(), &options) != 0) {
                LOG(FATAL) << "Fail to initialize channel";
                assert(false);
            }
            stub_.reset(new dbx1000::GlobalLockService_Stub(&channel_));
            Test();
        }

        RC GlobalLockServiceClient::LockRemote(int instance_id, uint64_t page_id, LockMode req_mode, char *page_buf, size_t count) {
//            cout << "GlobalLockServiceClient::LockRemote page_id : " << page_id << ", count : " << count << endl;
            dbx1000::LockRemoteRequest request;
            dbx1000::LockRemoteReply reply;
            ::brpc::Controller cntl;

            request.set_instance_id(instance_id);
            request.set_page_id(page_id);
            request.set_req_mode(GlobalLockServiceHelper::SerializeLockMode(req_mode));
            request.set_count(count);

            stub_->LockRemote(&cntl, &request, &reply, NULL);
            if (!cntl.Failed()) {
                RC rc = GlobalLockServiceHelper::DeSerializeRC(reply.rc());
                if(RC::TIME_OUT == rc) {
                    return RC::TIME_OUT;
                }
                if(RC::Abort == rc) {
                    return RC::Abort;
                }
                assert(RC::RCOK == rc);
                // 要是 server 数据版本比当前新，返回最新的版本
                if(count > 0) {
                    assert(MY_PAGE_SIZE == count);
                    assert(reply.count() == count);
                    memcpy(page_buf, reply.page_buf().data(), count);
                }
                return RC::RCOK ;
            } else {
                LOG(FATAL) << cntl.ErrorText();
                assert(false);
                return RC::Abort;
            }
        }

        void GlobalLockServiceClient::InstanceInitDone(int instance_id) {
            dbx1000::InstanceInitDoneRequest request;
            dbx1000::InstanceInitDoneReply reply;
            ::brpc::Controller cntl;

            request.set_instance_id(instance_id);

            stub_->InstanceInitDone(&cntl, &request, &reply, nullptr);
            if (!cntl.Failed()) { } else {
                LOG(FATAL) << cntl.ErrorText();
                assert(false);
            }
        }

        bool GlobalLockServiceClient::GlobalLockInitDone() {
            dbx1000::GlobalLockInitDoneRequest request;
            dbx1000::GlobalLockInitDoneReply reply;
            ::brpc::Controller cntl;

            stub_->GlobalLockInitDone(&cntl, &request, &reply, nullptr);
            if (!cntl.Failed()) {
                return reply.init_done();
            } else {
                LOG(FATAL) << cntl.ErrorText();
                assert(false);
            }
        }

        uint64_t GlobalLockServiceClient::GetNextTs() {
            dbx1000::GetNextTsRequest request;
            dbx1000::GetNextTsReply reply;
            ::brpc::Controller cntl;

            stub_->GetNextTs(&cntl, &request, &reply, nullptr);
            if (!cntl.Failed()) {
                return reply.ts();
            } else {
                LOG(FATAL) << cntl.ErrorText();
                assert(false);
            }
        }

        RC GlobalLockServiceClient::Invalid(uint64_t page_id, char *page_buf, size_t count){
            dbx1000::InvalidRequest request;
            dbx1000::InvalidReply reply;
            ::brpc::Controller cntl;

            request.set_page_id(page_id);
            request.set_count(count);

            stub_->Invalid(&cntl, &request, &reply, nullptr);
            if (!cntl.Failed()) {
                RC rc = GlobalLockServiceHelper::DeSerializeRC(reply.rc());
                if(RC::TIME_OUT == rc) {
                    return RC::TIME_OUT;
                }
                assert(RC::RCOK == rc);
                if(count > 0) {
                    assert(MY_PAGE_SIZE == count);
                    assert(reply.count() == count);
                    memcpy(page_buf, reply.page_buf().data(), count);
                }
                return RC::RCOK ;
            } else {
                LOG(FATAL) << cntl.ErrorText();
                assert(false);
                return RC::Abort;
            }
        }

        int GlobalLockServiceClient::Test() {
            dbx1000::TestRequest request;
            dbx1000::TestReply reply;
            ::brpc::Controller cntl;

            stub_->Test(&cntl, &request, &reply, nullptr);
            if (!cntl.Failed()) {
                return 0;
            } else {
                LOG(FATAL) << cntl.ErrorText();
                return -1;
            }
        }









        void GlobalLockServiceImpl::Start( const std::string &host) {
//            brpc::Server server;

            if (this->server.AddService(this, brpc::SERVER_DOESNT_OWN_SERVICE) != 0) {
                LOG(FATAL) << "Fail to add service";
                assert(false);
            }

            brpc::ServerOptions options;
            if (this->server.Start(std::stoi(host.substr(host.find(':')+1)), &options) != 0) {
                LOG(FATAL) << "Fail to start GlobalLockServiceImpl";
                assert(false);
            }
//            this->server.RunUntilAskedToQuit();
        }

        void GlobalLockServiceImpl::Invalid(::google::protobuf::RpcController* controller,
                               const ::dbx1000::InvalidRequest* request,
                               ::dbx1000::InvalidReply* response,
                               ::google::protobuf::Closure* done) {
//            cout << "other want to invalid page " << request->page_id() << ", count : " << request->count() << endl;
            ::brpc::ClosureGuard done_guard(done);
            ::brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);

            size_t count = request->count();
            if(count > 0) { assert(request->count() == MY_PAGE_SIZE); }
            else { assert(0 == count); }
            char page_buf[MY_PAGE_SIZE];
            assert(RC::RCOK == manager_instance_->lock_table()->LockInvalid(request->page_id(), page_buf, count));
            if(request->page_id() == 0) { cout << "something."; assert(manager_instance_->lock_table()->lock_table_[request->page_id()]->lock_mode == dbx1000::LockMode::O);}
            response->set_page_buf(page_buf, count);
            response->set_count(count);
            response->set_rc(RpcRC::RCOK);
        }

        void GlobalLockServiceImpl::LockRemote(::google::protobuf::RpcController* controller,
                               const ::dbx1000::LockRemoteRequest* request,
                               ::dbx1000::LockRemoteReply* response,
                               ::google::protobuf::Closure* done) {
//            std::cout << "GlobalLockServiceImpl::LockRemote, instance_id : " << request->instance_id() << ", page_id : " << request->page_id() << ", count : " << request->count() << std::endl;
            ::brpc::ClosureGuard done_guard(done);
            ::brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);

            RC rc;
            char *page_buf = nullptr;
            size_t count = request->count();
            if(count > 0) {
                assert(MY_PAGE_SIZE == count);
                page_buf = new char [MY_PAGE_SIZE];
            }
            else {assert(0 == count);}

            rc = global_lock_->LockRemote(request->instance_id(), request->page_id(), page_buf, count);

            if(RC::TIME_OUT  == rc) {
                response->set_rc(RpcRC::TIME_OUT);
            }
            if(RC::Abort  == rc) {
                response->set_rc(RpcRC::Abort);
                return;
            }
            assert(RC::RCOK == rc);
            response->set_rc(RpcRC::RCOK);
            if(count > 0) {
                response->set_page_buf(page_buf, count);
            }
            response->set_count(count);
        }

        /*
        ::grpc::Status GlobalLockServiceImpl::UnLockRemote(::grpc::ServerContext* context, const ::dbx1000::UnLockRemoteRequest* request, ::dbx1000::UnLockRemoteReply* response) {

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

        void GlobalLockServiceImpl::InstanceInitDone(::google::protobuf::RpcController* controller,
                               const ::dbx1000::InstanceInitDoneRequest* request,
                               ::dbx1000::InstanceInitDoneReply* response,
                               ::google::protobuf::Closure* done) {
            ::brpc::ClosureGuard done_guard(done);
            ::brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);

            global_lock_->set_instance_i(request->instance_id());
        }

        void GlobalLockServiceImpl::GlobalLockInitDone(::google::protobuf::RpcController* controller,
                               const ::dbx1000::GlobalLockInitDoneRequest* request,
                               ::dbx1000::GlobalLockInitDoneReply* response,
                               ::google::protobuf::Closure* done) {
            ::brpc::ClosureGuard done_guard(done);
            ::brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
            response->set_init_done(global_lock_->init_done());
        }

        void GlobalLockServiceImpl::GetNextTs(::google::protobuf::RpcController* controller,
                               const ::dbx1000::GetNextTsRequest* request,
                               ::dbx1000::GetNextTsReply* response,
                               ::google::protobuf::Closure* done) {
            ::brpc::ClosureGuard done_guard(done);
            ::brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
            response->set_ts(global_lock_->GetNextTs(-1));
        }

        void GlobalLockServiceImpl::Test(::google::protobuf::RpcController* controller,
                       const ::dbx1000::TestRequest* request,
                       ::dbx1000::TestReply* response,
                       ::google::protobuf::Closure* done){
            ::brpc::ClosureGuard done_guard(done);
            ::brpc::Controller *cntl = static_cast<brpc::Controller *>(controller);
        }
    }
}