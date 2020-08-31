#include "global.h"

//#include "stats.h"
//#include "manager.h"
//#include "lock_server/lock_server_table.h"
//#include "instance/manager_instance.h"
//#include "query.h"
//#include "dl_detect.h"
//#include "plock.h"
//#include "occ.h"
//#include "vll.h"

namespace dbx1000_cf {

}

namespace dbx1000_instance {

}


//mem_alloc mem_allocator;
//dbx1000::Stats stats;
//DL_detect dl_detector;
//Manager * glob_manager;
//dbx1000::ManagerServer * glob_manager_server;
//dbx1000::ManagerClient * glob_manager_client;
//dbx1000::ApiTxnClient* api_txn_client;
//dbx1000::ApiConCtlClient* api_con_ctl_client;
//std::string txn_thread_host;
//uint64_t process_id;
//Query_queue * query_queue;
//Plock part_lock_man;
//OptCC occ_man;
#if CC_ALG == VLL
VLLMan vll_man;
#endif 

//! wl 加载完成，所有数据都加载到内存里了
bool volatile warmup_finish = false;
bool volatile enable_thread_mem_pool = false;
pthread_barrier_t warmup_bar;
#ifndef NOGRAPHITE
carbon_barrier_t enable_barrier;
#endif

ts_t g_abort_penalty = ABORT_PENALTY;
bool g_central_man = CENTRAL_MAN;
uint32_t g_ts_alloc = TS_ALLOC;                   //! 对 ts 变量上锁的方式，有 mutex,还有原子自增
bool g_key_order = KEY_ORDER;
bool g_no_dl = NO_DL;
//ts_t g_timeout = TIMEOUT;
ts_t g_timeout = 1000000;
ts_t g_dl_loop_detect = DL_LOOP_DETECT;
bool g_ts_batch_alloc = TS_BATCH_ALLOC;
uint32_t g_ts_batch_num = TS_BATCH_NUM;           //! ts 整体加上的数，默认 1

bool g_part_alloc = PART_ALLOC;
bool g_mem_pad = MEM_PAD;
uint32_t g_cc_alg = CC_ALG;
ts_t g_query_intvl = QUERY_INTVL;
uint32_t g_part_per_txn = PART_PER_TXN;
double g_perc_multi_part = PERC_MULTI_PART;
double g_read_perc = READ_PERC;
double g_write_perc = WRITE_PERC;
double g_zipf_theta = ZIPF_THETA;
bool g_prt_lat_distr = PRT_LAT_DISTR;
uint32_t g_part_cnt = PART_CNT;                   //! 数据分区数
uint32_t g_virtual_part_cnt = VIRTUAL_PART_CNT;
uint32_t g_thread_cnt = THREAD_CNT;               //! 主线程数
uint64_t g_synth_table_size = SYNTH_TABLE_SIZE;
uint32_t g_req_per_query = REQ_PER_QUERY;
uint32_t g_field_per_tuple = FIELD_PER_TUPLE;
uint32_t g_init_parallelism = INIT_PARALLELISM;   //! wl 加载时，并行度

uint32_t g_num_wh = NUM_WH;
double g_perc_payment = PERC_PAYMENT;
bool g_wh_update = WH_UPDATE;
char * output_file = NULL;

map<string, string> g_params;

#if TPCC_SMALL
uint32_t g_max_items = 10000;
uint32_t g_cust_per_dist = 2000;
#else 
uint32_t g_max_items = 100000;
uint32_t g_cust_per_dist = 3000;
#endif
