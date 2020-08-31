//
// Created by rrzhang on 2020/5/29.
//
#include <cassert>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include "index.h"

#include "config.h"
#include "common/storage/disk/file_io.h"
#include "util/profiler.h"

namespace dbx1000 {

    IndexItem::IndexItem(uint64_t page_id, uint64_t location) {
        page_id_ = page_id;
        page_location_ = location;
    }
    IndexItem::IndexItem() {}

    Index::Index(const std::string& index_name) : index_name_(index_name) {}

    Index::~Index() {
//        Serialize();
        auto iter = index_.begin();
        while(index_.end() != iter) {
            delete iter->second;
            iter++;
        }
    }

    IndexFlag Index::IndexGet(uint64_t key, IndexItem* indexItem) {
        auto iter = index_.find(key);
        if (index_.end() == iter) {
            return IndexFlag::NOT_EXIST;
        }
        if (index_.end() != iter) {
            indexItem->page_id_ = iter->second->page_id_;
            indexItem->page_location_ = iter->second->page_location_;
            return IndexFlag::EXIST;
        }
    }

    int Index::IndexPut(uint64_t key, IndexItem* indexItem) {
        mutex_.lock();
        auto iter = index_.find(key);
        if (index_.end() != iter) {
            iter->second->page_id_ = indexItem->page_id_;
            iter->second->page_location_ = indexItem->page_location_;
            mutex_.unlock();
            return 0;
        }
        if (index_.end() == iter) {
            index_.insert(std::pair<uint64_t, IndexItem *>(key, new IndexItem(indexItem->page_id_, indexItem->page_location_)));
            mutex_.unlock();
            return 0;
        }

        return -1;
    }

    void Index::Serialize() {
        std::cout << "Index::Serialize..." << std::endl;
        Profiler profiler;
        profiler.Start();

        int fd = FileIO::Open(std::string(DB_PREFIX) + this->index_name_);
        assert(fd >= 0);
        char buf[524280];
        uint64_t buf_used_size = 0, offset = 0;
        uint64_t index_size = index_.size();
        uint64_t file_size = sizeof(uint64_t) + sizeof(uint64_t) + 3 * sizeof(uint64_t) * index_.size();
        FileIO::Write(fd, &file_size, sizeof(uint64_t), 0);
        FileIO::Write(fd, &index_size, sizeof(uint64_t), sizeof(uint64_t));
        offset += 2 * sizeof(uint64_t);

        for(auto iter : index_) {
            if(buf_used_size == 524280) {
                size_t x = FileIO::Write(fd, buf, buf_used_size, offset);
                assert(x == buf_used_size);
                offset += buf_used_size;
                buf_used_size = 0;
            }
            uint64_t key = iter.first;
            memcpy(&buf[buf_used_size + 0 * sizeof(uint64_t)], &key, sizeof(uint64_t));
            memcpy(&buf[buf_used_size + 1 * sizeof(uint64_t)], &iter.second->page_id_, sizeof(uint64_t));
            memcpy(&buf[buf_used_size + 2 * sizeof(uint64_t)], &iter.second->page_location_, sizeof(uint64_t));
            buf_used_size += (3 * sizeof(uint64_t));
        }
        if(buf_used_size > 0) {
            pwrite(fd, buf, buf_used_size, offset);
        }
        assert(FileIO::Close(fd) == 0);
        profiler.End();
        std::cout << "Index::Serialize done, time : " << profiler.Millis() << " millis."<< std::endl;
    }

    void Index::DeSerialize() {
        Profiler profiler;
        profiler.Start();
        std::cout << "Index::DeSerialize..." << std::endl;

        int fd = FileIO::Open(std::string(DB_PREFIX) + this->index_name_);
        assert(fd >= 0);
        uint64_t file_size;
        uint64_t offset = 0;
        uint64_t index_size;
        FileIO::Read(fd, &file_size, sizeof(uint64_t), 0);
        offset += sizeof(uint64_t);
        FileIO::Read(fd, &index_size, sizeof(uint64_t), sizeof(uint64_t));
        offset += sizeof(uint64_t);

        char buf[524280];
        while(offset < file_size) {
            size_t read_size = (offset + 524280 <= file_size) ? 524280 : (file_size - offset);
            size_t x = FileIO::Read(fd, buf, read_size, offset);
            assert(x == read_size);
            offset += read_size;

            IndexItem* indexItem = new IndexItem();
            for(int i = 0; i < read_size / (3 * sizeof(uint64_t)); i++) {
                uint64_t key;
                memcpy(&key, &buf[3 * i * sizeof(uint64_t) + 0 * sizeof(uint64_t)], sizeof(uint64_t));
                memcpy(&indexItem->page_id_, &buf[3 * i * sizeof(uint64_t) + 1 * sizeof(uint64_t)], sizeof(uint64_t));
                memcpy(&indexItem->page_location_, &buf[3 * i * sizeof(uint64_t) + 2 * sizeof(uint64_t)], sizeof(uint64_t));
                IndexPut(key, indexItem);
            }
            delete indexItem;
        }
        assert(FileIO::Close(fd) == 0);
        profiler.End();
        std::cout << "Index::DeSerialize done, time : " << profiler.Millis() << " millis."<< std::endl;
    }

    void Index::Serialize2() {
        Profiler profiler;
        profiler.Start();
        std::cout << "Index::Serialize2..." << std::endl;
        std::ostringstream oss;
        auto iter = index_.begin();
        while(index_.end() != iter) {
            oss << iter->first << " " << iter->second->page_id_ << " " << iter->second->page_location_ << " ";
            iter++;
        }
        std::ofstream out(std::string(DB_PREFIX) + index_name_, std::ios::out | std::ios::binary);
        out << oss.str();
        out.close();
        profiler.End();
        std::cout << "Index::Serialize2 done, time : " << profiler.Millis() << " millis."<< std::endl;
    }

    void Index::DeSerialize2() {
        Profiler profiler;
        profiler.Start();
        std::cout << "Index::DeSerialize2..." << std::endl;
        std::ifstream in(std::string(DB_PREFIX) + index_name_, std::ios::in | std::ios::binary);
        std::stringstream ss;
        ss << in.rdbuf();
//        std::cout << ss.str() << std::endl;

        uint64_t key;
        uint64_t page_id;
        uint64_t locatin;
        while(ss >> key){
            ss >> page_id;
            ss >> locatin;
            index_.insert(std::pair<uint64_t , IndexItem*>(key, new IndexItem(page_id, locatin)));
        }
        in.close();
        profiler.End();
        std::cout << "Index::DeSerialize2 done, time : " << profiler.Millis() << " millis."<< std::endl;
    }


    const std::string& Index::index_name() { return index_name_; }
    void Index::set_index_name(const std::string& index_name) { this->index_name_ = index_name; }

    void Index::Print() {
        auto iter = index_.begin();
        while(index_.end() != iter) {
            std::cout << "key:" << iter->first << ", page_id:" << iter->second->page_id_ << ", page_location:" << iter->second->page_location_ << std::endl;
            iter++;
        }
    }
}