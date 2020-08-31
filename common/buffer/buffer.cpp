//
// Created by rrzhang on 2020/4/9.
//

#include <cassert>
#include <cstring>
#include <string>
#include <sys/mman.h>
#include "buffer.h"

#include "lru_index.h"
#include "lru.h"
#include "common/storage/disk/file_io.h"
#include "common/storage/tablespace/page.h"
//#include "instance/manager_instance.h"
#include "shared_disk_service.h"

namespace dbx1000 {

    Buffer::Buffer(uint64_t total_size, size_t page_size, SharedDiskClient *sharedDiskClient)
            : total_size_(total_size)
              , page_size_(page_size)
              , page_num_(total_size / page_size)
              , shared_disk_client_(sharedDiskClient){
        ptr_ = mmap(NULL, total_size_, PROT_READ | PROT_WRITE, MAP_ANON | MAP_SHARED, -1, 0);
        page_list_ = new LRU(page_size_);
        free_list_ = new LRU(page_size_);
        lru_index_ = new LruIndex();

        /// initital free list
        for (int i = 0; i < page_num_; i++) {
            PageNode* page_node = new PageNode((char *) ptr_ + i * page_size_);
            free_list_->Prepend(page_node);
        }
        assert(free_list_->size() == page_num_);
        assert(page_list_->size() == 0);
    }

    Buffer::~Buffer() {
        FlushALl();
        delete free_list_;
        delete page_list_;
        delete lru_index_;
        if(nullptr != ptr_) {
            munmap(ptr_, total_size_);
        }
    }

    void Buffer::FlushPageList() {
        assert(free_list_->size() == 0);
        assert(page_list_->size() == page_num_);

        /// 只刷链表后 1/5 的数据
        for (int i = 0; i < page_num_ / 5; i++) {
            PageNode* page_node = page_list_->Popback();
            /// 写盘
            #ifdef SHARED_DISK
            shared_disk_client_->WritePage(page_node->page_->page_id(), page_node->page_->page_buf());
            #else
            FileIO::WritePage(page_node->page_->page_id(), page_node->page_->page_buf());
            #endif
            lru_index_->IndexDelete(page_node->page_->page_id());
            page_node->page_->set_page_id(UINT64_MAX);
            free_list_->Prepend(page_node);
        }
    }
    
    void Buffer::FlushALl() {
        PageNode* page_node;
        int size = page_list_->size();
        for (int i = 0; i < size; i++) {
            page_node = page_list_->Popback();
            /// 写盘
            #ifdef SHARED_DISK
            shared_disk_client_->WritePage(page_node->page_->page_id(), page_node->page_->page_buf());
            #else
            FileIO::WritePage(page_node->page_->page_id(), page_node->page_->page_buf());
            #endif
            lru_index_->IndexDelete(page_node->page_->page_id());
            page_node->page_->set_page_id(UINT64_MAX);
            free_list_->Prepend(page_node);
        }
    }

    void Buffer::FreeListToPageList(uint64_t page_id, const void* page_buf, size_t count) {
        assert(free_list_->size() > 0);
        assert(page_list_->size() < page_num_);

        PageNode *page_node = free_list_->Popback();
        page_node->page_->set_page_id(page_id);
        page_node->page_->set_page_buf(page_buf, count);
        lru_index_->IndexPut(page_id, page_node);
        page_list_->Prepend(page_node);
    }

    int Buffer::BufferGetWithLock(uint64_t page_id, void *buf, size_t count) {
        std::lock_guard<std::mutex> lock(mutex_);
        assert(page_list_->size() + free_list_->size() == page_num_);

        LruIndexFlag flag;
        PageNode *page_node = lru_index_->IndexGet(page_id, &flag);

        /// page in buffer
        if(LruIndexFlag::EXIST == flag) {
            page_list_->Get(page_node);
            assert(nullptr != page_node->page_->page_buf());
            memcpy(buf, page_node->page_->page_buf(), count);
            return 0;
        }
        /// page in disk
        if(LruIndexFlag::NOT_EXIST == flag) {
            /// 读盘
            #ifdef SHARED_DISK
            shared_disk_client_->ReadPage(page_id, buf);
            #else
            FileIO::ReadPage(page_id, buf);
            #endif
            if (free_list_->size() <= 0) {
                FlushPageList();
            }
            FreeListToPageList(page_id, buf, count);
            return 0;
        }

        assert(false);      /// 不应该到达这一步
    }

    int Buffer::BufferPutWithLock(uint64_t page_id, const void* buf, size_t count) {
        std::lock_guard<std::mutex> lock(mutex_);
        assert(page_list_->size() + free_list_->size() == page_num_);

        LruIndexFlag flag;
        PageNode *page_node = lru_index_->IndexGet(page_id, &flag);

        /// page in buffer
        if (LruIndexFlag::EXIST == flag) {
            assert(nullptr != page_node);
            page_list_->Get(page_node);
            page_node->page_->set_page_buf(buf, count);
            return 0;
        }
        if(LruIndexFlag::NOT_EXIST == flag) {
            assert(nullptr == page_node);
            if(free_list_->size() <= 0) {
                FlushPageList();
            }
            FreeListToPageList(page_id, buf, count);
            return 0;
        }

        assert(false);      /// 不应该到达这一步
    }


    int Buffer::BufferGet(uint64_t page_id, void *buf, size_t count) {
        mutex_.lock();
        assert(page_list_->size() + free_list_->size() == page_num_);
        mutex_.unlock();

        /// 找 page_node 的索引
        LruIndexFlag flag;
        PageNode *page_node = lru_index_->IndexGet(page_id, &flag);

        /// page in buffer
        if(LruIndexFlag::EXIST == flag) {
            mutex_.lock();
            page_list_->Get(page_node);
            mutex_.unlock();
            assert(nullptr != page_node->page_->page_buf());
            memcpy(buf, page_node->page_->page_buf(), count);
            return 0;
        }
        /// page in disk
        if(LruIndexFlag::NOT_EXIST == flag) {
            /// 读盘
            #ifdef SHARED_DISK
            shared_disk_client_->ReadPage(page_id, buf);
            #else
            FileIO::ReadPage(page_id, buf);
            #endif
            mutex_.lock();
            if (free_list_->size() <= 0) {
                FlushPageList();
            }
            FreeListToPageList(page_id, buf, count);
            mutex_.unlock();
            return 0;
        }

        assert(false);      /// 不应该到达这一步
    }

    int Buffer::BufferPut(uint64_t page_id, const void* buf, size_t count) {
        mutex_.lock();
        assert(page_list_->size() + free_list_->size() == page_num_);
        mutex_.unlock();

        /// 找 page_node 的索引
        LruIndexFlag flag;
        PageNode *page_node = lru_index_->IndexGet(page_id, &flag);

        /// page in buffer
        if (LruIndexFlag::EXIST == flag) {
            mutex_.lock();
            page_list_->Get(page_node);
            mutex_.unlock();
            page_node->page_->set_page_buf(buf, count);
            return 0;
        }
        if(LruIndexFlag::NOT_EXIST == flag) {
            mutex_.lock();
            if(free_list_->size() <= 0) {
                FlushPageList();
            }
            FreeListToPageList(page_id, buf, count);
            mutex_.unlock();
            return 0;
        }

        assert(false);      /// 不应该到达这一步
    }

    void Buffer::Print(){
        std::cout << "total_size:" << total_size_ << ", page_size:" << page_size_ << ", page_num_" << page_num_ << std::endl;
        lru_index_->Print();
    }
}