//
// Created by rrzhang on 2020/4/27.
//

#ifndef DBX1000_MYHELPER_H
#define DBX1000_MYHELPER_H

#include <chrono>
#include "global.h"

namespace dbx1000 {
    class MyHelper {
    public:
        static int RCToInt(RC rc);
        static RC IntToRC(int i);
        static int AccessToInt(access_t type);
        static access_t IntToAccess(int i);
        static int TsTypeToInt(TsType ts_type);
        static TsType IntToTsType(int i);
        static std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> GetSysClock();
    };
}

#define M_ASSERT(cond, ...) \
	if (!(cond)) {\
		printf("ASSERTION FAILURE [%s : %d] ", \
		__FILE__, __LINE__); \
		printf(__VA_ARGS__);\
		assert(false);\
	}

#define ASSERT(cond) assert(cond)

#define INC_STATS(tid, name, value) \
	if (STATS_ENABLE) \
		stats._stats[tid]->name += value;

#define INC_TMP_STATS(tid, name, value) \
	if (STATS_ENABLE) \
		stats.tmp_stats[tid]->name += value;

#define INC_GLOB_STATS(name, value) \
	if (STATS_ENABLE) \
		stats.name += value;

#define ATOM_CAS(dest, oldval, newval) \
	__sync_bool_compare_and_swap(&(dest), oldval, newval)


#define COMPILER_BARRIER asm volatile("" ::: "memory");
#define PAUSE { __asm__ ( "pause;" ); }



#endif //DBX1000_MYHELPER_H
