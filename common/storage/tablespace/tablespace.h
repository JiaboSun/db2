//
// Created by rrzhang on 2020/6/1.
//

#ifndef STORAGE_TABLESPACE_H
#define STORAGE_TABLESPACE_H

#include <cstdint>
#include <string>
#include <mutex>

namespace dbx1000 {
    class IndexItem;

    class TableSpace {
    public:
        TableSpace();
        TableSpace(const std::string& );
        ~TableSpace();

        uint64_t GetNextPageId();
        uint64_t GetLastPageId() const;
        void Serialize() const;
        void DeSerialize();

        void Print();

        /// getter and setter
        void set_table_name_(const std::string& );
        void set_table_size(uint64_t);
        void set_page_size(uint64_t);
        void set_row_size(uint64_t);
        void set_last_page_id(uint64_t);
        const std::string& table_name() const;
        uint64_t table_size() const;
        uint64_t page_size() const;
        uint64_t row_size() const;
        uint64_t last_page_id() const;

    private:
        std::string table_name_;
        uint64_t table_size_;       /// in bytes
        uint64_t page_size_;        /// in bytes
        uint64_t row_size_;
        int64_t last_page_id_;
        std::mutex mtx_;
    };
}

#endif //STORAGE_TABLESPACE_H
