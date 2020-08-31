//
// Created by rrzhang on 2020/6/1.
//
#include <cstring>
#include "memory_io.h"

#include "config.h"

namespace dbx1000 {
    std::unordered_map<uint64_t, std::string> MemoryIO::db_;

    int MemoryIO::Write(uint64_t page_id, const void *page_buf) {
        auto iter = db_.find(page_id);
        if (db_.end() != iter) {
            db_.erase(iter);
        }
        db_.insert(std::pair<uint64_t, std::string>(page_id, std::string((char *) page_buf, MY_PAGE_SIZE)));
        return 0;
    }

    int MemoryIO::Read(uint64_t page_id, void *page_buf) {
        if (db_.end() == db_.find(page_id)) {
            return -1;
        }
        memcpy(page_buf, db_[page_id].data(), MY_PAGE_SIZE);
        return 0;
    }
}