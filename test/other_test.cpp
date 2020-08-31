//
// Created by rrzhang on 2020/6/3.
//

//
#include <iostream>
#include <memory>
#include <thread>
#include <vector>
#include <map>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include <util/profiler.h>

using namespace std;

void Test_access() {
    cout << ((access("../db/", F_OK) >= 0) ? "exist" : "not exist") << endl;
    cout << ((access("../db/MAIN_TABLE", F_OK) >= 0) ? "exist" : "not exist")
         << endl;
}

map<uint64_t, char *> GetMap() {
    map<uint64_t, char *> str_map;
    return str_map;
}

void Test_map() {
    map<uint64_t, char *> map_2 = GetMap();
    map_2.insert(std::pair<uint64_t, char *>(1, const_cast<char *>("sdf")));
}

enum class LockMode {
    O,  /// 失效
    S,  /// 读锁
    X,  /// 写锁
};
struct LockNode {
    LockMode lock;
    mutex mtx;
    std::condition_variable cv;
    int count;
};

bool Lock(LockNode *lockNode, LockMode mode, int thead_id) {
    if (mode == LockMode::X) {
        std::unique_lock<std::mutex> lck(lockNode->mtx);
        while (lockNode->lock != LockMode::O) {
            cout << "thread " << thead_id << " lock X waiting..." << endl;
            lockNode->cv.wait(lck);
        }
        assert(lockNode->count == 0);
        lockNode->lock = LockMode::X;
        lockNode->count++;
        return true;
    }
    if (mode == LockMode::S) {
        std::unique_lock<std::mutex> lck(lockNode->mtx);
        while (lockNode->lock == LockMode::X) {
            cout << "thread " << thead_id << " lock S waiting..." << endl;
            lockNode->cv.wait(lck);
        }
        assert(lockNode->count >= 0);
        lockNode->lock = LockMode::S;
        lockNode->count++;
        return true;
    }
    assert(false);
}

bool UnLock(LockNode *lockNode) {
    std::unique_lock<std::mutex> lck(lockNode->mtx);
    if (LockMode::X == lockNode->lock) {
        assert(lockNode->count == 1);
        lockNode->count--;
        lockNode->lock = LockMode::O;
//        lockNode->cv.notify_one();
        lockNode->cv.notify_all();
        return true;
    }
    if (LockMode::S == lockNode->lock) {
        lockNode->count--;
        assert(lockNode->count >= 0);
        lockNode->lock = LockMode::S;
        if (lockNode->count == 0) {
            lockNode->lock = LockMode::O;
        }
//        lockNode->cv.notify_one();
        lockNode->cv.notify_all();
        return true;
    }
//    assert(false);
}

void Write(LockNode *lockNode, int thread_index) {
    Lock(lockNode, LockMode::X, thread_index);
    cout << "thread " << thread_index << " lock X success" << endl;
    this_thread::sleep_for(std::chrono::seconds(1));
    UnLock(lockNode);
    cout << "thread " << thread_index << " unlock success" << endl;
};

void Read(LockNode *lockNode, int thread_index) {
    Lock(lockNode, LockMode::S, thread_index);
    cout << "thread " << thread_index << " lock S success" << endl;
    this_thread::sleep_for(std::chrono::seconds(1));
    UnLock(lockNode);
    cout << "thread " << thread_index << " unlock success" << endl;
};

void TestLock() {
    dbx1000::Profiler profiler;
    profiler.Start();
    LockNode *lockNode = new LockNode();
    lockNode->count = 0;
    lockNode->lock = LockMode::O;
    Lock(lockNode, LockMode::X, 0);

    vector<thread> lock_threads;
    for (int i = 0; i < 10; i++) {
        lock_threads.emplace_back(thread(Read, lockNode, i + 1));
    }
    for (int i = 10; i < 20; i++) {
        lock_threads.emplace_back(thread(Write, lockNode, i + 1));
    }
    UnLock(lockNode);
    for (int i = 0; i < 20; i++) {
        lock_threads[i].join();
    }

    profiler.End();
    cout << "exe time : " << profiler.Seconds() << endl;
}

void Test_sizeof() {
    cout << "INT32_MAX : " << INT32_MAX << endl;
    cout << "UINT32_MAX : " << UINT32_MAX << endl;
    cout << "INT64_MAX : " << INT64_MAX << endl;
    cout << "UINT64_MAX : " << UINT64_MAX << endl;
    {
        // off_t 为 int64
        off_t offset = UINT64_MAX;
        assert(-1 == offset);
    }
    {
        // size_t 为 uint64
        size_t size = -1;
        assert(size == UINT64_MAX);
    }
}

void Test_cv_wait_for() {
    mutex mtx;
    condition_variable cv;
    int a = 0;
    bool flag = false;

    std::thread thread1([&]() {
        std::unique_lock<std::mutex> lck(mtx);
        cout << "thread1 start" << endl;
        this_thread::sleep_for(chrono::seconds(5));
        flag = true;
        cout << "thread1 end" << endl;
//        std::notify_all_at_thread_exit(cv, std::move(lck));
        cv.notify_all();
    });
//    this_thread::sleep_for(chrono::seconds(2));

    cout << "start threads" << endl;
    vector<thread> threads;
    for (int i = 0; i < 5; i++) {
        threads.emplace_back(thread([&]() {
            std::unique_lock<std::mutex> lck(mtx);
/*
            std::cv_status status = cv.wait_for(lck, chrono::seconds(3));
            if (std::cv_status::timeout == status) {
                cout << "timeout" << endl;
                cv.notify_all();
                return;
            }
            if (std::cv_status::no_timeout == status) {
                a++;
                return;
            }*/
//            if (cv.wait_for(lck, chrono::seconds(0), [flag]() { return flag; })) {
            if (cv.wait_until(lck, chrono::steady_clock::now() + chrono::seconds(1), [flag]() { return flag; })) {
                a++;
            } else {
                cout << "timeout" << endl;
            }
        }));
    }
    for (int i = 0; i < 5; i++) {
        threads[i].join();
    }
    thread1.join();
    cout << a << endl;
}

int main() {
//    Test_access();
//    Test_map();
//    TestLock();
//    Test_sizeof();
    Test_cv_wait_for();
    return 0;
}