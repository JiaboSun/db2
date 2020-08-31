//
// Created by rrzhang on 2020/6/3.
//

#include <iostream>
#include <vector>
#include <thread>
#include <cassert>
#include "lock_table.h"
#include "util/profiler.h"
#include "util/string_util.h"

using namespace std;

#define lock_table_test_num (10000)
#define test_thread_num (30)

/// 定义一个全为 0 的数组，几个线程并发对所有的位置都加 ++，最后验证是否所有的值都为线程数
void Test_Lock_Table() {
    dbx1000::LockTable *lockTable = new dbx1000::LockTable();
    lockTable->Init(0, lock_table_test_num, 0);
    dbx1000::Profiler profiler;

    int a[lock_table_test_num];
    for (int i = 0; i < lock_table_test_num; i++) { a[i] = 0; }

    vector<thread> threads_write;
    /// write && read
    profiler.Start();
    for (int i = 0; i < test_thread_num; i++) {
        threads_write.emplace_back(thread(
                [&]() {
                    for (uint64_t j = 0; j < lock_table_test_num; j++) {
                        assert(RC::RCOK == lockTable->Lock(j, dbx1000::LockMode::X));
                        a[j]++;
                        assert(RC::RCOK == lockTable->UnLock(j));
                    }
                }
        ));
    }
    for (int i = 0; i < 20; i++) {
        threads_write.emplace_back(thread(
                [&]() {
                    for (uint64_t j = 0; j < lock_table_test_num; j++) {
                        assert(RC::RCOK == lockTable->Lock(j, dbx1000::LockMode::S));
                        assert(RC::RCOK == lockTable->UnLock(j));
                    }
                }
        ));
    }
    for (int i = 0; i < test_thread_num + 20; i++) {
        threads_write[i].join();
    }
    profiler.End();
    cout << test_thread_num << " threads write time : " << profiler.Nanos() << " nanos." << endl;

    /// read
    profiler.Clear();
    profiler.Start();
    vector<thread> threads_read;
    for (int i = 0; i < test_thread_num; i++) {
        threads_read.emplace_back(thread(
                [&]() {
                    for (uint64_t j = 0; j < lock_table_test_num; j++) {
                        assert(RC::RCOK == lockTable->Lock(j, dbx1000::LockMode::S));
                        assert(a[j] == test_thread_num);
                        assert(RC::RCOK == lockTable->UnLock(j));
                    }
                }
        ));
    }
    for (int i = 0; i < test_thread_num; i++) {
        threads_read[i].join();
    }
    profiler.End();
    cout << test_thread_num << " threads write time : " << profiler.Nanos() << " nanos." << endl;

    delete lockTable;
}
