//
// Created by rrzhang on 2020/6/1.
//
#include <fstream>
#include <iostream>
#include <cassert>
#include "tablespace.h"

#include "config.h"
#include "json/json.h"

namespace dbx1000 {
    TableSpace::TableSpace() :
            table_size_(0)
            , page_size_(MY_PAGE_SIZE)
            , row_size_(0)
            , last_page_id_(-1) {}

    TableSpace::TableSpace(const std::string &table_name) :
            table_name_(table_name)
            , table_size_(0)
            , page_size_(MY_PAGE_SIZE)
            , row_size_(0)
            , last_page_id_(-1) {}

    TableSpace::~TableSpace() {}

    uint64_t TableSpace::GetNextPageId() {
        mtx_.lock();
        last_page_id_++;
        table_size_ += MY_PAGE_SIZE;
        mtx_.unlock();
        return last_page_id_;
    }

    uint64_t TableSpace::GetLastPageId() const { return last_page_id_; }

    void TableSpace::Serialize() const {
        std::ofstream out(std::string(DB_PREFIX) + this->table_name_, std::ios::out | std::ios::binary);
        /*
        std::ofstream out;
        out.open(std::string(DB_PREFIX) + this->table_name_);
         */
        assert(out.is_open());

        Json::Value root;
        Json::StreamWriterBuilder builder;
        const std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());

        root["table_name"] = this->table_name_;
        root["table_size"] = this->table_size_;
        root["page_size"] = this->page_size_;
        root["row_size"] = this->row_size_;
        root["last_page_id"] = this->last_page_id_;

        writer->write(root, &out);
        out.close();
    }

    void TableSpace::DeSerialize() {
        /*
        std::ifstream in(std::string(DB_PREFIX) + this->table_name_, std::ios::in | std::ios::binary);
        Json::Value root;
        in >> root;
        */
        std::ifstream in;
        in.open((std::string(DB_PREFIX) + this->table_name_));
        assert(in.is_open());
        Json::Value root;
        Json::CharReaderBuilder builder;
        builder["collectComments"] = true;
        JSONCPP_STRING errs;
        if (!parseFromStream(builder, in, &root, &errs)) {
            std::cout << errs << std::endl;
            assert(false);
//            return EXIT_FAILURE;
        }

        this->table_name_ = root["table_name"].asString();
        this->table_size_ = root["table_size"].asUInt64();
        this->page_size_ = root["page_size"].asUInt64();
        this->row_size_ = root["row_size"].asUInt64();
        this->last_page_id_ = root["last_page_id"].asInt64();

        in.close();
    }

    void TableSpace::Print() {
    std::cout << "table_name:" << table_name_ << ", table_size:" << table_size_
         << ", page_size:" << page_size_ << ", last_page_id:" << last_page_id_ << std::endl;
    }

    void TableSpace::set_table_name_(const std::string& table_name) { this->table_name_ = table_name; }
    void TableSpace::set_table_size(uint64_t table_size) { this->table_size_ = table_size; }
    void TableSpace::set_page_size(uint64_t page_size) { this->page_size_ = page_size; }
    void TableSpace::set_row_size(uint64_t row_size) { this->row_size_ = row_size; }
    void TableSpace::set_last_page_id(uint64_t last_page_id) { this->last_page_id_ = last_page_id; }
    const std::string& TableSpace::table_name() const { return this->table_name_; }
    uint64_t TableSpace::table_size() const { return this->table_size_; }
    uint64_t TableSpace::page_size() const { return this->page_size_; }
    uint64_t TableSpace::row_size() const { return this->row_size_; }
    uint64_t TableSpace::last_page_id() const { return this->last_page_id_; }
}