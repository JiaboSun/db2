#include <cmath>
#include <set>

#include "ycsb_query.h"
#include "util/arena.h"

uint64_t ycsb_query::the_n = 0;
double ycsb_query::denom = 0;

void ycsb_query::init(Query_thd* query_thd) {
    request_cnt = 0;
	_query_thd = query_thd;
    requests = new (_query_thd->queryQueue_->arenas_[_query_thd->thread_id_]->Allocate(sizeof(ycsb_request) * g_req_per_query)) ycsb_request[g_req_per_query]();
    part_to_access = new (_query_thd->queryQueue_->arenas_[_query_thd->thread_id_]->Allocate(sizeof(uint64_t) * g_req_per_query)) uint64_t();
	zeta_2_theta = zeta(2, g_zipf_theta);   //! 1.65975
	assert(the_n != 0);
	assert(denom != 0);
	gen_requests(_query_thd->thread_id_);
}

void ycsb_query::calculateDenom()
{
	assert(the_n == 0);
		/**
		 * 为了实现多个进程访问的数据没有重叠，没有冲突时，每个进程只访问 key:[g_synth_table_size/MAX_PROCESS_CNT*this->process_id, g_synth_table_size/MAX_PROCESS_CNT*(this->process_id+1)] 范围内的数据
		 * 但是，query 模块没有对应进程的 id, 所以产生的 key 范围只在 [0, g_synth_table_size/MAX_PROCESS_CNT]
		 * 在事务执行时，应该加上 : g_synth_table_size / MAX_PROCESS_CNT * process_id;
		 *
		 * 为什么是 MAX_PROCESS_CNT，而不是 PROCESS_CNT？
		 * PROCESS_CNT 下，随着 PROCESS_CNT 增大， key 范围缩小，会导致每个节点内事务同时访问的 page_id 的概率增大
		 * MAX_PROCESS_CNT 是预估最大达到的实例数
		 */
#ifdef DB2_WITH_NO_CONFLICT
    uint64_t table_size = (g_synth_table_size / g_virtual_part_cnt) / MAX_PROCESS_CNT;
#else
	uint64_t table_size = g_synth_table_size / g_virtual_part_cnt;      //! 1024*1024*10 / 1
#endif
	the_n = table_size - 1;
	denom = zeta(the_n, g_zipf_theta);
	//! denom = 1605.65, when n = 1024*1024*10 and theta = 0.6
}

// The following algorithm comes from the paper:
// Quickly generating billion-record synthetic databases
// However, it seems there is a small bug. 
// The original paper says zeta(theta, 2.0). But I guess it should be 
// zeta(2.0, theta).
//! n == 1024*1024*10 - 1, theta == 0.6
double ycsb_query::zeta(uint64_t n, double theta) {
	double sum = 0;
	for (uint64_t i = 1; i <= n; i++) 
		sum += pow(1.0 / i, theta);
	return sum;
	//! sum = 1605.65, when n = 1024*1024*10 and theta = 0.6
}
//! n == 1024*1024*10 - 1, theta == 0.6
//! 返回一个整数
uint64_t ycsb_query::zipf(uint64_t n, double theta) {
	assert(this->the_n == n);
	assert(theta == g_zipf_theta);
	double alpha = 1 / (1 - theta);
	double zetan = denom;
	//! eta == 0.998981
	double eta = (1 - pow(2.0 / n, 1 - theta)) / 
		(1 - zeta_2_theta / zetan);
	double u; 
	drand48_r(&_query_thd->buffer, &u);
	double uz = u * zetan;
	if (uz < 1) return 1;
	if (uz < 1 + pow(0.5, theta)) return 2;
	return 1 + (uint64_t)(n * pow(eta*u -eta + 1, alpha));
}
//! thd_id = 0, 1, 2, 3...
//! 为某个线程的 queries[i] 生成请求（16 个 requests），保证每个 request 的 primary 没有冲突
//! ，若 primary 有冲突，则 queries[i][16] 可能没有完全设置，即该 query 内 request 可能不足 16，数量由 ycsb_query::request_cnt 记录。
//! 最后返回的 queries[thd_id][request_cnt] 是按 row_t 里主键排好序的，scan 类型的只按第一个row_t 主键排序
void ycsb_query::gen_requests(int thd_id) {
#if CC_ALG == HSTORE
	assert(g_virtual_part_cnt == g_part_cnt);
#endif
	int access_cnt = 0;
	set<uint64_t> all_keys;
	part_num = 0;
	double r = 0;
	int64_t rint64 = 0;
	drand48_r(&_query_thd->buffer, &r);         //! 产生 [0.0, 1.0) 之间的随机数
	lrand48_r(&_query_thd->buffer, &rint64);    //! [0, 2^31)
	if (r < g_perc_multi_part) { //! g_perc_multi_part == 1
	    //! 该 for 语句应该是填充 part_to_access，该数组长度为 g_part_per_txn(1)
	    //! part_to_access[0] = thd_id % g_virtual_part_cnt
	    //! part_to_access[i](i > 0) 随机，且不超过 g_virtual_part_cnt
	    //! 并且各 part_to_access[i] 互不相等
		for (uint32_t i = 0; i < g_part_per_txn; i++) {                       //! g_part_per_txn == 1
			if (i == 0 && FIRST_PART_LOCAL)
				part_to_access[part_num] = thd_id % g_virtual_part_cnt;     //! g_virtual_part_cnt == 1
			else {
				part_to_access[part_num] = rint64 % g_virtual_part_cnt;
			}
			uint32_t j;
			for (j = 0; j < part_num; j++) 
				if ( part_to_access[part_num] == part_to_access[j] )
					break;
			if (j == part_num)
				part_num ++;
		}
	} else {
		part_num = 1;
		if (FIRST_PART_LOCAL)
			part_to_access[0] = thd_id % g_part_cnt;
		else
			part_to_access[0] = rint64 % g_part_cnt;
	}

	//! now, part_num == g_part_per_txn == 1
	//! part_to_access[0] == 0 or 1

	int rid = 0;
	for (uint32_t tmp = 0; tmp < g_req_per_query; tmp ++) {		       //! g_req_per_query == 16
		double r;
		drand48_r(&_query_thd->buffer, &r);
		ycsb_request * req = &requests[rid];
		//! 设置 读、写、查询 比例，读 0.9， 写 0.1，剩下的为查询（0.0）
		if (r < g_read_perc) {  //! g_read_perc == 0.9
			req->rtype = RD;
		} else if (r >= g_read_perc && r <= g_write_perc + g_read_perc) {
			req->rtype = WR;
		} else {
			req->rtype = SCAN;
			req->scan_len = SCAN_LEN;   //! SCAN_LEN == 20
		}

		// the request will access part_id.
		uint64_t ith = tmp * part_num / g_req_per_query;    //! 0
		uint64_t part_id = 
			part_to_access[ ith ];
		/**
		 * 为了实现多个进程访问的数据没有重叠，没有冲突时，每个进程只访问 key:[g_synth_table_size/MAX_PROCESS_CNT*this->process_id, g_synth_table_size/MAX_PROCESS_CNT*(this->process_id+1)] 范围内的数据
		 * 但是，query 模块没有对应进程的 id, 所以产生的 key 范围只在 [0, g_synth_table_size/MAX_PROCESS_CNT]
		 * 在事务执行时，应该加上 : g_synth_table_size / MAX_PROCESS_CNT * process_id;
		 *
		 * 为什么是 MAX_PROCESS_CNT，而不是 PROCESS_CNT？
		 * PROCESS_CNT 下，随着 PROCESS_CNT 增大， key 范围缩小，会导致每个节点内事务同时访问的 page_id 的概率增大
		 * MAX_PROCESS_CNT 是预估最大达到的实例数
		 */
 #ifdef DB2_WITH_NO_CONFLICT
    uint64_t table_size = (g_synth_table_size / g_virtual_part_cnt) / MAX_PROCESS_CNT;
#else
	uint64_t table_size = g_synth_table_size / g_virtual_part_cnt;      //! 1024*1024*10 / 1
#endif
		uint64_t row_id = zipf(table_size - 1, g_zipf_theta);   //! g_zipf_theta == 0.6
		assert(row_id < table_size);
		uint64_t primary_key = row_id * g_virtual_part_cnt + part_id;
		req->key = primary_key;
		int64_t rint64;
		lrand48_r(&_query_thd->buffer, &rint64);
		req->value = rint64 % (1<<8);
		// Make sure a single row is not accessed twice
		//! 任何操作的主键都不能一样，scan 查询的范围里，primary 也不能有任何重叠，access_cnt 记录去重后的 all_keys.size()
		if (req->rtype == RD || req->rtype == WR) {
			if (all_keys.find(req->key) == all_keys.end()) {
				all_keys.insert(req->key);
				access_cnt ++;
			} else continue;
		} else {
			bool conflict = false;
			for (uint32_t i = 0; i < req->scan_len; i++) {
				primary_key = (row_id + i) * g_part_cnt + part_id;
				if (all_keys.find( primary_key )
					!= all_keys.end())
					conflict = true;
			}
			if (conflict) continue;
			else {
				for (uint32_t i = 0; i < req->scan_len; i++)
					all_keys.insert( (row_id + i) * g_part_cnt + part_id);
				access_cnt += SCAN_LEN;
			}
		}
		rid ++;
	}
	request_cnt = rid;

	// Sort the requests in key order.
	if (g_key_order) {
		for (int i = request_cnt - 1; i > 0; i--) 
			for (int j = 0; j < i; j ++)
				if (requests[j].key > requests[j + 1].key) {
					ycsb_request tmp = requests[j];
					requests[j] = requests[j + 1];
					requests[j + 1] = tmp;
				}
		for (uint32_t i = 0; i < request_cnt - 1; i++)
			assert(requests[i].key < requests[i + 1].key);
	}
}


