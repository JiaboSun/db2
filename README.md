rpc 分支将事务线程和并发控制分开，事务线程可以在不同的机器上执行，但是只有一个集中式的并发控制。

事务线程相关的代码放在 client 下，包括 benchmarks 和 txn、thread, 还有一个 manager_client

server/ 文件夹下代码包括：buffer、并发控制、workload、manager_server等。

common/ 下代码为 server 和 client 共用。

system/ 下为原来DBx1000 的旧代码，用不到。

api/ 为 client/server 之间的接口，api/api_single_machine/ 为单机环境下的接口，rpc 和单机环境通过宏 WITH_RPC 控制。

util/ 下是一些工具包：
arena 提供内存分配
make_unique 创建只能指针
no_destructor 和 leveldb 相关
numbercomparator 为 leveldb 提供数字比较，leveldb 默认为字符串比较
profiler 为计时工具

global.cc 可能要调整的参数：
/// for server/workload
std::string g_schame_path = std::string("/home/zhangrongrong/CLionProjects/DBx1000/server/workload/YCSB_schema.txt");

/// for api/

/// for server/buffer/, buffer can use leveldb or memorydb
std::string g_db_path = std::string("/tmp/leveldb_for_dbx1000");



<img src="logo/dbx1000.svg" alt="DBx1000 Logo" width="40%">

-----------------

DBx1000 is a single node OLTP database management system (DBMS). The goal of DBx1000 is to make DBMS scalable on future 1000-core processors. We implemented all the seven classic concurrency control schemes in DBx1000. They exhibit different scalability properties under different workloads. 

The concurrency control scalability study is described in the following paper. 

    Staring into the Abyss: An Evaluation of Concurrency Control with One Thousand Cores
    Xiangyao Yu, George Bezerra, Andrew Pavlo, Srinivas Devadas, Michael Stonebraker
    http://www.vldb.org/pvldb/vol8/p209-yu.pdf
    
Build & Test
------------

To build the database.

    make -j

To test the database

    python test.py
    
Configuration
-------------

DBMS configurations can be changed in the config.h file. Please refer to README for the meaning of each configuration. Here we only list several most important ones. 

    THREAD_CNT        : Number of worker threads running in the database.
    WORKLOAD          : Supported workloads include YCSB and TPCC
    CC_ALG            : Concurrency control algorithm. Seven algorithms are supported 
                        (DL_DETECT, NO_WAIT, HEKATON, SILO, TICTOC) 
    MAX_TXN_PER_PART  : Number of transactions to run per thread per partition.
                        
Configurations can also be specified as command argument at runtime. Run the following command for a full list of program argument. 
    
    ./rundb -h

Run
---

The DBMS can be run with 

    ./rundb

Output
------

txn_cnt: The total number of committed transactions. This number is close to but smaller than THREAD_CNT * MAX_TXN_PER_PART. When any worker thread commits MAX_TXN_PER_PART transactions, all the other worker threads will be terminated. This way, we can measure the steady state throughput where all worker threads are busy.

abort_cnt: The total number of aborted transactions. A transaction may abort multiple times before committing. Therefore, abort_cnt can be greater than txn_cnt.

run_time: The aggregated transaction execution time (in seconds) across all threads. run_time is approximately the program execution time * THREAD_CNT. Therefore, the per-thread throughput is txn_cnt / run_time and the total throughput is txn_cnt / run_time * THREAD_CNT.

time_{wait, ts_alloc, man, index, cleanup, query}: Time spent on different components of DBx1000. All numbers are aggregated across all threads.

time_abort: The time spent on transaction executions that eventually aborted.

latency: Average latency of transactions.







