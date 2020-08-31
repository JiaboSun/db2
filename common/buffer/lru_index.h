//
// Created by rrzhang on 2020/4/14.
//

#ifndef DBX1000_LRU_INDEX_H
#define DBX1000_LRU_INDEX_H

#include <vector>

//#define USE_TBB

#ifdef USE_TBB
#include <tbb/concurrent_hash_map.h>
#else
#include <unordered_map>
#endif

namespace dbx1000 {
    class PageNode;

#ifdef USE_TBB
    typedef tbb::concurrent_hash_map<uint64_t, PageNode*> HashMap;
    typedef typename HashMap::accessor HashMapAccessor;
#endif
    enum class LruIndexFlag{
        EXIST,
        NOT_EXIST,
    };

    class LruIndex {
    public:
        PageNode* IndexGet(uint64_t page_id, LruIndexFlag *flag);
        void IndexPut(uint64_t page_id, PageNode* page_node);
        void IndexDelete(uint64_t page_id);

        void Print();
//    private:
#ifdef USE_TBB
        HashMap lru_map_;
#else
        std::unordered_map<uint64_t, PageNode*> lru_map_;
#endif
    };
} // namespace dbx1000


#endif //DBX1000_LRU_INDEX_H
