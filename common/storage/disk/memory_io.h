//
// Created by rrzhang on 2020/6/1.
//

#ifndef STORAGE_MEMORY_UO_H
#define STORAGE_MEMORY_UO_H

#include <unordered_map>
namespace dbx1000 {
    class MemoryIO {
        static int Write(uint64_t page_id, const void* page_buf);
        static int Read(uint64_t page_id, void* page_buf);
        static std::unordered_map<uint64_t, std::string> db_;
    };
}


#endif //STORAGE_MEMORY_UO_H
