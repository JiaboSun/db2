//
// Created by rrzhang on 2020/6/1.
//
#include <cassert>
#include <cstring>
#include <iostream>
#include "page.h"

#include "config.h"

namespace dbx1000 {

    Page::Page(char *buf) : page_id_(UINT64_MAX)
                            , page_buf_(buf)
                            , used_size_(64)
                            , page_size_(MY_PAGE_SIZE) {}

    Page::~Page() {
        delete page_buf_;
    }

    int Page::PagePut(uint64_t page_id, const char *row_buf, size_t count) {
        assert(this->page_id_ == page_id);
        assert(used_size_ + count <= page_size_);
        memcpy(&page_buf_[this->used_size_], row_buf, count);
        this->used_size_ += count;
    }

    Page* Page::Serialize() {
        memcpy(&page_buf_[0 * sizeof(uint64_t)], reinterpret_cast<void *>(&page_id_), sizeof(uint64_t));
        memcpy(&page_buf_[1 * sizeof(uint64_t)], reinterpret_cast<void *>(&page_size_), sizeof(uint64_t));
        memcpy(&page_buf_[2 * sizeof(uint64_t)], reinterpret_cast<void *>(&used_size_), sizeof(uint64_t));
        memcpy(&page_buf_[3 * sizeof(uint64_t)], reinterpret_cast<void *>(&version_), sizeof(uint64_t));
        return this;
    }

    void Page::Deserialize() {
        page_id_ = *(uint64_t*)(&page_buf_[0 * sizeof(uint64_t*)]);
        page_size_ = *(uint64_t*)(&page_buf_[1 * sizeof(uint64_t*)]);
        used_size_ = *(uint64_t*)(&page_buf_[2 * sizeof(uint64_t*)]);
        version_ = *(uint64_t*)(&page_buf_[3 * sizeof(uint64_t*)]);
    }

    void Page::Print() {
        std::cout << "page_id:" << page_id_ << ", page_size:" << page_size_ << ", used_size:" << used_size_
        << ", page_buf:" << std::string(&(page_buf_[64]), used_size_-64)
        << ", version:" << version_ << std::endl;
    }


    void Page::set_page_id(uint64_t page_id) { this->page_id_ = page_id; }
    void Page::set_page_buf(const void *page_buf, size_t count) { memcpy(this->page_buf_, page_buf, count); }
    void Page::set_page_size(uint64_t page_size) { this->page_size_ = page_size; }
    void Page::set_used_size(uint64_t used_size) {this->used_size_ = used_size; }
    void Page::set_version(uint64_t version) { this->version_ = version; }
    uint64_t Page::page_id() const { return this->page_id_; }
    char *Page::page_buf() { return this->page_buf_; }
    uint64_t Page::page_size() const { return this->page_size_; }
    uint64_t Page::used_size() const { return this->used_size_;}
    uint64_t Page::version() const { return this->version_; }
}