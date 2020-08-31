//
// Created by rrzhang on 2020/4/14.
//

#ifndef DBX1000_LRU_H
#define DBX1000_LRU_H


#include <atomic>
#include <mutex>

//#define USE_MUTEX
#ifndef USE_MUTEX
//#define USE_CAS
#endif

namespace dbx1000 {

    class LruIndex;
    class Page;

    class PageNode{
    public:
        PageNode();
        PageNode(char *);
        ~PageNode();

        Page* page_;
        PageNode *prev_;
        PageNode *next_;
    };

    class LRU {
    public:
        explicit LRU(int item_size);
        ~LRU();

        void Prepend(PageNode* );
        PageNode* Popback();
        void Get(PageNode* );

        void Check();

        /// getter and setter
        int size();

        PageNode* head() { return this->head_; }
        PageNode* tail() { return this->tail_; }

    private:

#ifdef USE_CAS
        std::atomic<PageNode*> head_;
        std::atomic<PageNode*> tail_;
#else
        PageNode* head_;
        PageNode* tail_;
#endif
        std::atomic_int size_;      /// length of this list
        std::mutex mtx_;
    };
} // namespace dbx1000

#endif //DBX1000_LRU_H
