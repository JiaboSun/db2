//
// Created by rrzhang on 2020/5/5.
//

#ifndef DBX1000_MYSTATS_H
#define DBX1000_MYSTATS_H

#include <memory>
#include "util/profiler.h"

//enum class RC;

namespace dbx1000 {
    class Stats_thd_rpc{
    public:

    };

    /// 每个 Stats_thd 对象只记录当前线程的信息
    class Stats_thd {
    public:
        void init(uint64_t thd_id);
        void clear();

        uint64_t thread_id_;
        uint64_t txn_cnt;            /// RCOK 的事务个数
        uint64_t abort_cnt;          /// Abort 的事务个数，可能该 Abort 事务在后面某个时间点 被重新执行为 RCOK
        uint64_t run_time;           /// 累加所有单个事务/query 的执行时间
        /// 以下三个对应 Stats_tmp 中几个成员
        uint64_t time_man;           ///
        uint64_t time_index;         ///
        uint64_t time_wait;          ///
        uint64_t time_abort;         /// 所有 abort txn 的耗时，作为 run_time 的子集
        uint64_t time_cleanup;       /// cleanup/return_row 的时间
        uint64_t time_ts_alloc;      /// 获取时间戳的耗时
        uint64_t time_query;         /// 为每个 txn 生成 m_query 的时间
        uint64_t wait_cnt;           ///
        uint64_t debug1;             ///
        uint64_t debug2;             ///
        uint64_t debug3;             /// Row_mvcc::access 执行的时间，包括 R_REQ、P_REQ、WR_REQ、XP_REQ、
        uint64_t debug4;             /// Row_mvcc::access 获取锁的时间
        uint64_t debug5;             ///

        uint64_t latency;
        uint64_t *all_debug1;
        uint64_t *all_debug2;
        /**
         * zhangrongrong, 2020/6/30
         */
         uint64_t time_remote_lock_;
         uint64_t count_remote_lock_;
         uint64_t count_total_request_;
         uint64_t count_write_request_;
//         uint64_t total_run_time_;
//         uint64_t total_latency_;
    };

    /// 事务执行有 thread -> txn
    /// 该类记录单个 txn 内的过程耗时
    /// 在 thread 执行完该 txn 的执行后，将该类耗时累加到 Stats_thd 中
    class Stats_tmp {
    public:
        void init();
        void clear();

        uint64_t time_man;      /// 对应 ycsb 中 get_row、return_row  时间
        uint64_t time_index;
        uint64_t time_wait;     /// 对应 ycsb 中 get_row 时等待的时间，time_man 的子集
        /**
         * zhangrongrong, 2020/6/30
         */
         uint64_t time_remote_lock_;
    };

    class Stats {
    public:
        // PER THREAD statistics
        Stats_thd **_stats;    //! 数组大小等于线程数
        // stats are first written to tmp_stats, if the txn successfully commits,
        // copy the values in tmp_stats to _stats
        Stats_tmp **tmp_stats;

        // GLOBAL statistics
        uint64_t dl_detect_time;
        uint64_t dl_wait_time;
        uint64_t cycle_detect;
        uint64_t deadlock;
        /**
         * zhangrongrong, 2020/6/30
         */
        uint64_t txn_cnt;

        void init();
        void init(uint64_t thread_id);
        void clear(uint64_t tid);
        void add_debug(uint64_t thd_id, uint64_t value, uint32_t select);
        void commit(uint64_t thd_id);
        void abort(uint64_t thd_id);
        void print(uint64_t ins_id);
        void print_rpc();
        void print_lat_distr();
    };
}


#endif //DBX1000_MYSTATS_H
