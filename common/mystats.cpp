//
// Created by rrzhang on 2020/5/5.
//
#include <iostream>
#include <iomanip>
#include "mystats.h"

#include "global.h"
#include "util/make_unique.h"
#include "util/parse_result.h"

//#define BILLION 1000000000UL
#define BILLION 1000UL
namespace dbx1000 {
    void Stats_thd::init(uint64_t thread_id) {
        thread_id_ = thread_id;
        clear();
        all_debug1 = new uint64_t[MAX_TXN_PER_PART]();
        all_debug2 = new uint64_t[MAX_TXN_PER_PART]();
    }

    void Stats_thd::clear() {
        txn_cnt = 0;
        abort_cnt = 0;
        run_time = 0;
        time_man = 0;
        time_index = 0;
        time_wait = 0;
        time_abort = 0;
        time_cleanup = 0;
        time_ts_alloc = 0;
        time_query = 0;
        wait_cnt = 0;
        debug1 = 0;
        debug2 = 0;
        debug3 = 0;
        debug4 = 0;
        debug5 = 0;
        latency = 0;

        /**
         * zhangrongrong, 2020/6/30
         */
         time_remote_lock_ = 0;
    }

    void Stats_tmp::init() {
        clear();
    }

    void Stats_tmp::clear() {
        time_man = 0;
        time_index = 0;
        time_wait = 0;
        time_remote_lock_ = 0;
    }

    void Stats::init() {
        if (!STATS_ENABLE)
            return;
        _stats = new Stats_thd*[g_thread_cnt]();
        tmp_stats = new Stats_tmp*[g_thread_cnt]();
        dl_detect_time = 0;
        dl_wait_time = 0;
        cycle_detect = 0;
        deadlock = 0;

        /**
         * zhangrongrong, 2020/6/30
         */
        txn_cnt = 0;
    }

//! 为成员变量分配空间
    void Stats::init(uint64_t thread_id) {
        if (!STATS_ENABLE)
            return;
        _stats[thread_id] = new Stats_thd();
        tmp_stats[thread_id] = new Stats_tmp();

        _stats[thread_id]->init(thread_id);
        tmp_stats[thread_id]->init();
    }

    void Stats::clear(uint64_t tid) {
        if (STATS_ENABLE) {
            _stats[tid]->clear();
            tmp_stats[tid]->clear();

            dl_detect_time = 0;
            dl_wait_time = 0;
            cycle_detect = 0;
            deadlock = 0;
        }
    }

    void Stats::add_debug(uint64_t thd_id, uint64_t value, uint32_t select) {
        if (g_prt_lat_distr && warmup_finish) {
            uint64_t tnum = _stats[thd_id]->txn_cnt;
            if (select == 1)
                _stats[thd_id]->all_debug1[tnum] = value;
            else if (select == 2)
                _stats[thd_id]->all_debug2[tnum] = value;
        }
    }

    void Stats::commit(uint64_t thd_id) {
        if (STATS_ENABLE) {
            _stats[thd_id]->time_man += tmp_stats[thd_id]->time_man;
            _stats[thd_id]->time_index += tmp_stats[thd_id]->time_index;
            _stats[thd_id]->time_wait += tmp_stats[thd_id]->time_wait;
            _stats[thd_id]->time_remote_lock_ += tmp_stats[thd_id]->time_remote_lock_;
            tmp_stats[thd_id]->clear();
        }
    }

    void Stats::abort(uint64_t thd_id) {
        if (STATS_ENABLE) {
            _stats[thd_id]->time_abort += tmp_stats[thd_id]->time_man;
            tmp_stats[thd_id]->clear();
        }
    }

    void Stats::print(uint64_t ins_id) {
        uint64_t total_txn_cnt = 0;
        uint64_t total_abort_cnt = 0;
        uint64_t total_run_time = 0;
        uint64_t total_latency = 0;
        uint64_t total_time_man = 0;
        uint64_t total_debug1 = 0;
        uint64_t total_debug2 = 0;
        uint64_t total_debug3 = 0;
        uint64_t total_debug4 = 0;
        uint64_t total_debug5 = 0;
        uint64_t total_time_index = 0;
        uint64_t total_time_abort = 0;
        uint64_t total_time_cleanup = 0;
        uint64_t total_time_wait = 0;
        uint64_t total_time_ts_alloc = 0;
        uint64_t total_time_query = 0;
        uint64_t total_time_remote_lock = 0;
        uint64_t total_count_remote_lock = 0;
        uint64_t total_count_write_request = 0;
        uint64_t total_count_total_request = 0;

        for (uint64_t tid = 0; tid < g_thread_cnt; tid ++) {
            total_txn_cnt += _stats[tid]->txn_cnt;
            total_abort_cnt += _stats[tid]->abort_cnt;
            total_run_time += _stats[tid]->run_time;
            total_latency += _stats[tid]->latency;
            total_time_man += _stats[tid]->time_man;
            total_debug1 += _stats[tid]->debug1;
            total_debug2 += _stats[tid]->debug2;
            total_debug3 += _stats[tid]->debug3;
            total_debug4 += _stats[tid]->debug4;
            total_debug5 += _stats[tid]->debug5;
            total_time_index += _stats[tid]->time_index;
            total_time_abort += _stats[tid]->time_abort;
            total_time_cleanup += _stats[tid]->time_cleanup;
            total_time_wait += _stats[tid]->time_wait;
            total_time_ts_alloc += _stats[tid]->time_ts_alloc;
            total_time_query += _stats[tid]->time_query;
            total_time_remote_lock += _stats[tid]->time_remote_lock_;
            total_count_remote_lock += _stats[tid]->count_remote_lock_;
            total_count_write_request += _stats[tid]->count_write_request_;
            total_count_total_request += _stats[tid]->count_total_request_;

            printf("[tid=%ld] txn_cnt=%ld,abort_cnt=%ld\n",
                tid,
                _stats[tid]->txn_cnt,
                _stats[tid]->abort_cnt
            );
        }
        this->txn_cnt = total_txn_cnt;
        cout << "all thread run time : " << total_run_time / BILLION << " us, average latency : " << total_latency / BILLION / total_txn_cnt << " us." << endl;
        cout << " get ts time : " << total_time_ts_alloc / BILLION << ", all thread time remote lock : " << total_time_remote_lock / BILLION << " us." << endl;
        cout << "total_count_remote_lock/total_count_write_request/total_count_total_request : " << total_count_remote_lock << "/" << total_count_write_request << "/" << total_count_total_request << endl;
        AppendLatency(total_latency / BILLION / total_txn_cnt, ins_id);
        AppendRemoteLockTime(total_time_remote_lock / BILLION, ins_id);
    }
/*
    void Stats::print() {

        uint64_t total_txn_cnt = 0;
        uint64_t total_abort_cnt = 0;
        double total_run_time = 0;
        double total_time_man = 0;
        double total_debug1 = 0;
        double total_debug2 = 0;
        double total_debug3 = 0;
        double total_debug4 = 0;
        double total_debug5 = 0;
        double total_time_index = 0;
        double total_time_abort = 0;
        double total_time_cleanup = 0;
        double total_time_wait = 0;
        double total_time_ts_alloc = 0;
        double total_latency = 0;
        double total_time_query = 0;
        for (uint64_t tid = 0; tid < g_thread_cnt; tid ++) {
            total_txn_cnt += _stats[tid]->txn_cnt;
            total_abort_cnt += _stats[tid]->abort_cnt;
            total_run_time += _stats[tid]->run_time;
            total_time_man += _stats[tid]->time_man;
            total_debug1 += _stats[tid]->debug1;
            total_debug2 += _stats[tid]->debug2;
            total_debug3 += _stats[tid]->debug3;
            total_debug4 += _stats[tid]->debug4;
            total_debug5 += _stats[tid]->debug5;
            total_time_index += _stats[tid]->time_index;
            total_time_abort += _stats[tid]->time_abort;
            total_time_cleanup += _stats[tid]->time_cleanup;
            total_time_wait += _stats[tid]->time_wait;
            total_time_ts_alloc += _stats[tid]->time_ts_alloc;
            total_latency += _stats[tid]->latency;
            total_time_query += _stats[tid]->time_query;

            printf("[tid=%ld] txn_cnt=%ld,abort_cnt=%ld\n",
                tid,
                _stats[tid]->txn_cnt,
                _stats[tid]->abort_cnt
            );
        }
        FILE * outf;
        if (output_file != NULL) {
            outf = fopen(output_file, "w");
            fprintf(outf, "[summary] txn_cnt=%ld, abort_cnt=%ld"
                ", run_time=%f, time_wait=%f, time_ts_alloc=%f"
                ", time_man=%f, time_index=%f, time_abort=%f, time_cleanup=%f, latency=%f"
                ", deadlock_cnt=%ld, cycle_detect=%ld, dl_detect_time=%f, dl_wait_time=%f"
                ", time_query=%f, debug1=%f, debug2=%f, debug3=%f, debug4=%f, debug5=%f\n",
                total_txn_cnt,
                total_abort_cnt,
                total_run_time / BILLION,
                total_time_wait / BILLION,
                total_time_ts_alloc / BILLION,
                (total_time_man - total_time_wait) / BILLION,
                total_time_index / BILLION,
                total_time_abort / BILLION,
                total_time_cleanup / BILLION,
                total_latency / BILLION / total_txn_cnt,
                deadlock,
                cycle_detect,
                dl_detect_time / BILLION,
                dl_wait_time / BILLION,
                total_time_query / BILLION,
                total_debug1, // / BILLION,
                total_debug2, // / BILLION,
                total_debug3, // / BILLION,
                total_debug4, // / BILLION,
                total_debug5 / BILLION
            );
            fclose(outf);
        }
        printf("[summary] txn_cnt=%ld, abort_cnt=%ld"
            ", run_time=%f, time_wait=%f, time_ts_alloc=%f"
            ", time_man=%f, time_index=%f, time_abort=%f, time_cleanup=%f, latency=%f"
            ", deadlock_cnt=%ld, cycle_detect=%ld, dl_detect_time=%f, dl_wait_time=%f"
            ", time_query=%f, debug1=%f, debug2=%f, debug3=%f, debug4=%f, debug5=%f\n",
            total_txn_cnt,
            total_abort_cnt,
            total_run_time / BILLION,
            total_time_wait / BILLION,
            total_time_ts_alloc / BILLION,
            (total_time_man - total_time_wait) / BILLION,
            total_time_index / BILLION,
            total_time_abort / BILLION,
            total_time_cleanup / BILLION,
            total_latency / BILLION / total_txn_cnt,
            deadlock,
            cycle_detect,
            dl_detect_time / BILLION,
            dl_wait_time / BILLION,
            total_time_query / BILLION,
            total_debug1 / BILLION,
            total_debug2, // / BILLION,
            total_debug3, // / BILLION,
            total_debug4, // / BILLION,
            total_debug5  // / BILLION
        );
        if (g_prt_lat_distr)
            print_lat_distr();
    }
*/
/*
    void Stats::print_rpc() {
        uint64_t total_txn_cnt = 0;
        uint64_t total_abort_cnt = 0;
        double total_run_time = 0;
        double total_time_man = 0;
        double total_debug1 = 0;
        double total_debug2 = 0;
        double total_debug3 = 0;
        double total_debug4 = 0;
        double total_debug5 = 0;
        double total_time_index = 0;
        double total_time_abort = 0;
        double total_time_cleanup = 0;
        double total_time_wait = 0;
        double total_time_ts_alloc = 0;
        double total_latency = 0;
        double total_time_query = 0;

        double total_time_man_rpc_count = 0;
        double total_time_man_rpc_time = 0;
        double total_time_abort_rpc_count = 0;
        double total_time_abort_rpc_time = 0;
        double total_time_ts_alloc_rpc_time = 0;
        uint64_t total_time_ts_alloc_rpc_count = 0;

//        for (uint64_t tid = 0; tid < g_thread_cnt; tid ++) {
// TODO
        for(uint64_t tid = 0; tid < 0 + 1; tid++) {
            total_txn_cnt += _stats[tid]->txn_cnt;
            total_abort_cnt += _stats[tid]->abort_cnt;
            total_run_time += _stats[tid]->run_time;
            total_time_man += _stats[tid]->time_man;
            total_debug1 += _stats[tid]->debug1;
            total_debug2 += _stats[tid]->debug2;
            total_debug3 += _stats[tid]->debug3;
            total_debug4 += _stats[tid]->debug4;
            total_debug5 += _stats[tid]->debug5;
            total_time_index += _stats[tid]->time_index;
            total_time_abort += _stats[tid]->time_abort;
            total_time_cleanup += _stats[tid]->time_cleanup;
            total_time_wait += _stats[tid]->time_wait;
            total_time_ts_alloc += _stats[tid]->time_ts_alloc;
            total_latency += _stats[tid]->latency;
            total_time_query += _stats[tid]->time_query;

            total_time_man_rpc_time += _stats[tid]->time_man_rpc_time;
            total_time_man_rpc_count += _stats[tid]->time_man_rpc_count;
            total_time_abort_rpc_time += _stats[tid]->time_abort_rpc_time;
            total_time_abort_rpc_count += _stats[tid]->time_abort_rpc_count;
            total_time_ts_alloc_rpc_time += _stats[tid]->time_ts_alloc_rpc_time;
            total_time_ts_alloc_rpc_count += _stats[tid]->time_ts_alloc_rpc_count;

            cout << "[tid=" << _stats[tid]->txn_cnt
            << " txn_cnt=" << _stats[tid]->txn_cnt << ",abort_cnt=" << _stats[tid]->abort_cnt << endl;
        }
        cout << fixed << "[summary] txn_cnt=" << total_txn_cnt
        << ", abort_cnt=" << total_abort_cnt << endl
        << ", run_time_total=" << total_run_time / BILLION
        << ", run_time_actual=" << (total_run_time-total_time_man_rpc_time-total_time_abort_rpc_time-total_time_ts_alloc_rpc_time) / BILLION
        << ", run_time_latency=" << (total_time_man_rpc_time+total_time_abort_rpc_time+total_time_ts_alloc_rpc_time) / BILLION
        << ", run_time_latency_average=" << (total_time_man_rpc_time+total_time_abort_rpc_time+total_time_ts_alloc_rpc_time) / BILLION / total_time_man_rpc_count << endl
        << ", time_index=" << total_time_index / BILLION
        << ", time_abort_total=" << total_time_abort / BILLION
        << ", time_abort_actual=" << (total_time_abort-total_time_abort_rpc_time) / BILLION
        << ", time_abort_latency=" << (total_time_abort_rpc_time) / BILLION
        << ", time_abort_latency_average=" << (total_time_abort_rpc_time) / BILLION / total_time_abort_rpc_count << endl
        << ", time_wait=" << total_time_wait / BILLION << endl
        << ", time_ts_alloc_total=" << (total_time_ts_alloc + total_time_ts_alloc_rpc_time) / BILLION
        << ", time_ts_alloc_actual=" << (total_time_ts_alloc) / BILLION
        << ", time_ts_alloc_latency=" << total_time_ts_alloc_rpc_time / BILLION
        << ", time_ts_alloc_latency_average=" << total_time_ts_alloc_rpc_time / BILLION / total_time_ts_alloc_rpc_count
        << ", time_man_total=" << (total_time_man - total_time_wait) / BILLION
        << ", time_man_actual=" << (total_time_man - total_time_wait - total_time_man_rpc_time) / BILLION
        << ", time_man_latency=" << (total_time_man_rpc_time) / BILLION
        << ", time_man_latency_average=" << (total_time_man_rpc_time) / BILLION / total_time_man_rpc_count << endl
        << ", time_cleanup=" << total_time_cleanup / BILLION
        << ", latency=" << total_latency / BILLION / total_txn_cnt
        << ", time_query=" << total_time_query / BILLION
        << ", debug1=" << total_debug1 / BILLION << ", debug2=" << total_debug2 << ", debug3=" << total_debug3 << ", debug4=" << total_debug4 << ", debug5=" << total_debug5 << endl;
        if (g_prt_lat_distr)
            print_lat_distr();
    }
*/
    void Stats::print_lat_distr() {
        FILE *outf;
        if (output_file != NULL) {
            outf = fopen(output_file, "a");
            for (uint32_t tid = 0; tid < g_thread_cnt; tid++) {
                fprintf(outf, "[all_debug1 thd=%d] ", tid);
                for (uint32_t tnum = 0; tnum < _stats[tid]->txn_cnt; tnum++)
                    fprintf(outf, "%ld,", _stats[tid]->all_debug1[tnum]);
                fprintf(outf, "\n[all_debug2 thd=%d] ", tid);
                for (uint32_t tnum = 0; tnum < _stats[tid]->txn_cnt; tnum++)
                    fprintf(outf, "%ld,", _stats[tid]->all_debug2[tnum]);
                fprintf(outf, "\n");
            }
            fclose(outf);
        }
    }

} // namespace dbx1000