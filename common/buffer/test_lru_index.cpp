//
// Created by rrzhang on 2020/7/15.
//


#include <iostream>
#include <cstring>
#include <vector>
#include <thread>
#include <mutex>
#include <cassert>
#include "lru.h"
#include "lru_index.h"
#include "common/storage/tablespace/page.h"

#include "util/profiler.h"

using namespace std;


void Test_lru(){
    dbx1000::LruIndex *lruIndex = new dbx1000::LruIndex();
    dbx1000::Profiler profiler;

    profiler.Clear();
    profiler.Start();
    vector<thread> threads;
    int thread_num = 10;
    int total_item = 100000;
    mutex mtx;
    /// IndexPut
    for (int i = 0; i < thread_num; i++) {
        threads.emplace_back(thread(
                [total_item, thread_num, i, &lruIndex, &mtx]() {
                    for (uint64_t j = i*total_item/thread_num; j < (i+1)*total_item/thread_num; j++) {
                        lruIndex->IndexPut(j, new dbx1000::PageNode());
                    }
                }
        ));
    }
    for (auto& th:threads) { th.join(); }
    profiler.End();
    cout << "IndexPut  time : " << profiler.Micros() << endl;
    cout << "size : " << lruIndex->lru_map_.size() << endl;

    /// IndexGet
    threads.clear();
    profiler.Clear();
    profiler.Start();
    for (int i = 0; i < thread_num; i++) {
        threads.emplace_back(thread(
                [total_item, thread_num, i, &lruIndex, &mtx]() {
                    for (uint64_t j = 0; j < total_item; j++) {
                        lruIndex->IndexGet(j, new dbx1000::LruIndexFlag());
                    }
                }
        ));
    }
    for (auto& th:threads) { th.join(); }
    profiler.End();
    cout << "IndexGet  time : " << profiler.Micros() << endl;


    /// IndexDelete
    threads.clear();
    profiler.Clear();
    profiler.Start();
    for (int i = 0; i < thread_num; i++) {
        threads.emplace_back(thread(
                [total_item, thread_num, i, &lruIndex, &mtx]() {
                    for (uint64_t j = i*total_item/thread_num; j < (i+1)*total_item/thread_num; j++) {
                        lruIndex->IndexDelete(j);
                    }
                }
        ));
    }
    for (auto& th:threads) { th.join(); }
    profiler.End();
    cout << "IndexDelete  time : " << profiler.Micros() << endl;
    cout << "size : " << lruIndex->lru_map_.size() << endl;



    delete lruIndex;
}
int main(){

Test_lru();
//test ();
    return 0;
}//
// Created by rrzhang on 2020/7/16.
//

