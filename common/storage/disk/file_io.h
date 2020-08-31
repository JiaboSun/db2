//
// Created by rrzhang on 2020/6/1.
//

#ifndef STORAGE_FILE_IO_H
#define STORAGE_FILE_IO_H

#include <map>
namespace dbx1000 {

    class FileIO {
    public:
        static int Open(const std::string& path);
        static size_t Write(int fd, const void* buf, size_t size, uint64_t offset);
        static size_t Read(int fd, void* buf, size_t size, uint64_t offset);
        static size_t WritePage(uint64_t page_id, const void* page_buf);
        static size_t ReadPage(uint64_t page_id, void* page_buf);
        static int Close();
        static int Close(int fd);
        static std::map<std::string, int> opened_file_;
        static std::string db_name_;
    };
}

#endif //STORAGE_FILE_IO_H
