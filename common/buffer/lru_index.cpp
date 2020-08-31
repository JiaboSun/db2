//
// Created by rrzhang on 2020/4/14.
//
#include <iostream>
#include <cassert>
#include "lru_index.h"

namespace dbx1000 {
    PageNode* LruIndex::IndexGet(uint64_t page_id, LruIndexFlag *flag) {
#ifdef USE_TBB
        HashMapAccessor accessor;
        bool res = lru_map_.find(accessor, page_id);
        if(!res) {
            *flag = LruIndexFlag::NOT_EXIST;
            return nullptr;
        }
        *flag = LruIndexFlag::EXIST;
        assert(nullptr != accessor->second);
        return accessor->second;
#else
        auto iter = lru_map_.find(page_id);
        if (lru_map_.end() == iter) {
            *flag = LruIndexFlag::NOT_EXIST;
            return nullptr;
        }
        *flag = LruIndexFlag::EXIST;
        assert(nullptr != iter->second);
        return iter->second;
#endif
    }

    void LruIndex::IndexPut(uint64_t page_id, PageNode* page_node) {
        IndexDelete(page_id);
        lru_map_.insert(std::pair<uint64_t, PageNode*>(page_id, page_node));
    }

    void LruIndex::IndexDelete(uint64_t page_id) {
        lru_map_.erase(page_id);
    }

    void LruIndex::Print() {
        std::cout << "lru_index : {" << std::endl;
        int count = 0;
        for (auto &iter : lru_map_) {
            if (count % 10 == 0) { std::cout << "    "; }
            std::cout << ", {key:" << iter.first << ", page:" << iter.second << "}";
            count++;
            if (count % 10 == 0) { std::cout << std::endl; }
        }
        std::cout << std::endl << "}" << std::endl;
    }
}