//
// Created by rrzhang on 2020/6/1.
//
#include <cassert>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include "file_io.h"

#include "config.h"

namespace dbx1000 {
    std::string FileIO::db_name_;
    std::map<std::string, int> FileIO::opened_file_;

    int FileIO::Open(const std::string& path) {
        int fd = open(path.data(), O_RDWR | O_CREAT, 0644);
        assert(fd > 0);
        if(opened_file_.end() == opened_file_.find(path)) {
            opened_file_.insert(std::pair<std::string, int>(path, fd));
        }
        return fd;
    }
    size_t FileIO::Write(int fd, const void* buf, size_t size, uint64_t offset) {
        return pwrite(fd, buf, size, offset);
    }

    size_t FileIO::Read(int fd, void* buf, size_t size, uint64_t offset) {
        return pread(fd, buf, size, offset);
    }

    size_t FileIO::WritePage(uint64_t page_id, const void* page_buf) {
        std::string path("");
        path += DB_PREFIX;
        path += std::to_string(page_id / ITEM_NUM_PER_FILE);
        path += DB_SUFIX;
        int fd;
        auto iter = opened_file_.find(path);
        if(opened_file_.end() != iter) {
            fd = iter->second;
        } else {
            fd = Open(path);
        }
        assert(fd > 0);
        return pwrite(fd, page_buf, MY_PAGE_SIZE, (page_id % ITEM_NUM_PER_FILE) * MY_PAGE_SIZE);
    }

    size_t FileIO::ReadPage(uint64_t page_id, void* page_buf) {
        std::string path("");
        path += DB_PREFIX;
        path += std::to_string(page_id / ITEM_NUM_PER_FILE);
        path += DB_SUFIX;
        int fd;
        auto iter = opened_file_.find(path);
        if(opened_file_.end() != iter) {
            fd = iter->second;
        } else {
            fd = Open(path);
        }
        assert(fd > 0);
        return pread(fd, page_buf, MY_PAGE_SIZE, (page_id % ITEM_NUM_PER_FILE) * MY_PAGE_SIZE);
    }

    int FileIO::Close() {
        int res = 0;
        for(auto iter : opened_file_) {
            res = close(iter.second);
            if(res < 0) {
                return res;
            }
            opened_file_.erase(iter.first);
        }
        return res;
    }

    int FileIO::Close(int fd) {
        int res = close(fd);
        for (auto iter : opened_file_) {
            if (fd == iter.second) {
                opened_file_.erase(iter.first);
            }
        }
        return res;
    }

}