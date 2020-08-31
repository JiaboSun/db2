//
// Created by rrzhang on 2020/4/9.
//

#ifndef DBX1000_MAKE_UNIQUE_H
#define DBX1000_MAKE_UNIQUE_H

namespace dbx1000 {
    template<typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args &&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}

#endif //DBX1000_MAKE_UNIQUE_H
