//
// Created by rrzhang on 2019/9/12.
//

#ifndef COMPARE_GLUSTER_AND_GPRC_STRING_UTIL_H
#define COMPARE_GLUSTER_AND_GPRC_STRING_UTIL_H

#include <stdarg.h>
#include <cstring>
#include <thread>
#include <random>
#include <iostream>

class RandNum_generator {
private:
    RandNum_generator(const RandNum_generator &) = delete;

    RandNum_generator &operator=(const RandNum_generator &) = delete;

    std::uniform_int_distribution<unsigned> u;
    std::default_random_engine e;
    int mStart, mEnd;
public:
    // [start, end], inclusive, uniformally distributed
    RandNum_generator(int start, int end)
            : u(start, end)
              , e(std::hash<std::thread::id>()(std::this_thread::get_id()))
              , mStart(start), mEnd(end) {}

    // [mStart, mEnd], inclusive
    unsigned nextNum() {
        return u(e);
    }

    // [0, max], inclusive
    unsigned nextNum(unsigned max) {
        return unsigned((u(e) - mStart) / float(mEnd - mStart) * max);
    }
};

class StringUtil {
public:
    static std::string Random_string(RandNum_generator &rng,std::size_t strLen) {
        std::string rs(strLen, ' ');
        for (auto &ch : rs) {
            ch = rng.nextNum();
        }
        return rs;
    }
};


/// how to use :
//#include <iostream>
//#include "string_util.h"
//using namespace std;
//int main(){
//
//    RandNum_generator rng('a', 'z');
//    cout << StringUtil::Random_string(rng, 10) << endl;
//    cout << StringUtil::Random_string(rng, 10) << endl;
//    cout << StringUtil::Random_string(rng, 10) << endl;
//    cout << StringUtil::Random_string(rng, 10) << endl;
//    cout << StringUtil::Random_string(rng, 10) << endl;
//    return 0;
//}

#endif //COMPARE_GLUSTER_AND_GPRC_STRING_UTIL_H
