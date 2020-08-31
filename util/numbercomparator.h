//
// Created by rrzhang on 2020/4/5.
//

#ifndef DBX1000_NUMBERCOMPARATOR_H
#define DBX1000_NUMBERCOMPARATOR_H

#include "leveldb/comparator.h"
#include "leveldb/slice.h"

namespace dbx1000 {

    class NumberComparatorImpl : public leveldb::Comparator {
    public:
//        NumberComparatorImpl() = default;

        const char *Name() const { return "leveldb.NumberComparator"; }

        int Compare(const leveldb::Slice &a, const leveldb::Slice &b) const {
            if (a.size() == b.size()) {
                return a.compare(b);
//                return memcmp(a.data(), b.data(), a.size());
            } else {
                return a.size() > b.size() ? +1 : -1;
            }
        }

        void FindShortestSeparator(std::string *start, const leveldb::Slice &limit) const {}

        void FindShortSuccessor(std::string *key) const {}
    };
}


#endif //DBX1000_NUMBERCOMPARATOR_H
