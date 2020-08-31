//
// Created by rrzhang on 2020/5/29.
//

#ifndef STORAGE_INDEX_H
#define STORAGE_INDEX_H

#include <unordered_map>
#include <mutex>

namespace dbx1000 {

    enum class IndexFlag {
        EXIST,
        NOT_EXIST,
    };

    struct IndexItem {
    public:
        IndexItem(uint64_t page_id, uint64_t location);
        IndexItem();
        IndexItem(const IndexItem &) = delete;
        IndexItem &operator=(const IndexItem &) = delete;

        uint64_t page_id_;
        uint64_t page_location_;
    };

    class Index {
    public:
        Index(const std::string&);
        ~Index();

        IndexFlag IndexGet(uint64_t key, IndexItem* indexItem);
        int IndexPut(uint64_t key, IndexItem* indexItem);
        /// json 序列化
        bool ToJson();
        bool FromJson();
        /// uint64_t 转 char*, 再写入文件，文件大小比 json 小一个数量级，时间和 json 差不多
        void Serialize();
        void DeSerialize();
        /// 直接写入 stringstream，再往文件写，文件大小和 file 差不多，但是时间小一个数量级
        void Serialize2();
        void DeSerialize2();

        void Print();
        const std::string& index_name();
        void set_index_name(const std::string&);

    private:
        std::unordered_map<uint64_t, IndexItem *> index_;
        std::string index_name_;
        std::mutex mutex_;
    };
}

#endif //STORAGE_INDEX_H
