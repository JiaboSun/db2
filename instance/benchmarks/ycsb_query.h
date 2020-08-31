#ifndef _YCSB_QUERY_H_
#define _YCSB_QUERY_H_


#include <cassert>
#include "common/global.h"
#include "instance/benchmarks/query.h"

class workload;
class Query_thd;
// Each ycsb_query contains several ycsb_requests, 
// each of which is a RD, WR to a single table

class ycsb_request {
public:
	access_t rtype; 
	uint64_t key;
	char value;
	// only for (qtype == SCAN)
	uint32_t scan_len;
};

class ycsb_query : public base_query {
public:
	void init(int thread_id) { assert(false); };
//	void init(int thread_id, Query_thd * query_thd);
	void init(Query_thd * query_thd);
	static void calculateDenom();       //! 生成 denom

	uint64_t request_cnt;
	ycsb_request * requests;

private:
    //! 生成 requests
	void gen_requests(int thread_id);
	// for Zipfian distribution
	static double zeta(uint64_t n, double theta);       //! 计算 denom
	uint64_t zipf(uint64_t n, double theta);
	
	static uint64_t the_n;      //! table size - 1, 1024*1024*10 -1
	static double denom;        //! denom = 1605.65, when n = 1024*1024*10 and theta = 0.6
	double zeta_2_theta;        //! 1.65975
	Query_thd *_query_thd;
};

#endif
