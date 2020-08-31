//
// Created by rrzhang on 2020/4/9.
//

#ifndef DBX1000_BUFFER_H
#define DBX1000_BUFFER_H

#include <cstdlib>
#include <string>
#include <unordered_map>
#include <memory>
#include <iostream>
#include <mutex>
#include "config.h"

/**
 * buffer 使用 mmap 从系统申请一块大空间，划分成 MY_PAGE_SIZE 大小使用。
 * 初始化时给定 mmap 空间大小，和 MY_PAGE_SIZE 大小，
 * 两个链表 page_list_ 和 free_list_ 管理页面，同时使用 LruIndex 来加速 page_list_ 的查找
 * page_list_ 写满时，会把链表后 1/5 数据刷盘
 */
namespace dbx1000 {
    class LRU;
    class LruIndex;
    class ManagerClient;
    class SharedDiskClient;

    class Buffer {
    public:
        Buffer(uint64_t total_size, size_t page_size, SharedDiskClient *sharedDiskClient = nullptr);
        ~Buffer();

        /// 有锁读写，使用时直接锁住整个函数，相当于串行调度
        int BufferGetWithLock(uint64_t page_id, void* buf, size_t count);
        int BufferPutWithLock(uint64_t page_id, const void* buf, size_t count);
        /// 无锁读写，使用前应保证从锁表获取了页面的权限，这样就可以并行调用，只需要在函数内部保护好链表的操作就行
        /// 速度比有锁提升 2-3 倍
        int BufferGet(uint64_t page_id, void* buf, size_t count);
        int BufferPut(uint64_t page_id, const void* buf, size_t count);

        void Print();
        void FlushALl();

    private:
        /// 将 page_list_ 尾部部分数据刷到磁盘，这些节点被放到 free_list_ 中
        void FlushPageList();
        /// 从 free_list_ 头部取出一个节点放到 page_list_ 中，同时设置节点的 key_ 和 page_
        void FreeListToPageList(uint64_t page_id, const void* page, size_t count);

        void *ptr_;                          /// 内存池，存放 page 数据
        uint64_t total_size_;               /// size of ptr, in bytes
        size_t page_size_;
        int page_num_;                      /// number of pages, == total_size_ / page_size_

        LRU* page_list_;     /// 被使用的链表
        LRU* free_list_;    /// 空闲链表
        LruIndex* lru_index_;   /// page_list_ 的索引，根据 key, 直接定位到相应的 RowNode
        SharedDiskClient *shared_disk_client_;

        std::mutex mutex_;
    };
}

#endif //DBX1000_BUFFER_H
