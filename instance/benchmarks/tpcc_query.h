#ifndef _TPCC_QUERY_H_
#define _TPCC_QUERY_H_

#include "common/global.h"
#include "helper.h"
#include "instance/benchmarks/query.h"

class workload;

// items of new order transaction
struct Item_no {
	uint64_t ol_i_id;			//订单商品id
	uint64_t ol_supply_w_id;	//供货仓库id
	uint64_t ol_quantity;		//数量
};

class tpcc_query : public base_query {
public:
	void init(uint64_t thd_id, workload * h_wl);
	TPCCTxnType type;	//TPCC事务类型：支付操作、新订单、订单状态查询、发货、库存状态查询
	/**********************************************/	
	// common txn input for both payment & new-order
	/**********************************************/	
	uint64_t w_id;		//仓库id
	uint64_t d_id;		//区域id
	uint64_t c_id;		//顾客id
	/**********************************************/	
	// txn input for payment
	/**********************************************/	
	uint64_t d_w_id;	//区域仓库id	
	uint64_t c_w_id;	//顾客仓库id
	uint64_t c_d_id;	//顾客区域id
	char c_last[LASTNAME_LEN];	//顾客姓名
	double h_amount;
	bool by_last_name;	//是否使用的姓名
	/**********************************************/	
	// txn input for new-order
	/**********************************************/
	Item_no * items;
	bool rbk;		//是否回滚
	bool remote;	//是否远程
	uint64_t ol_cnt;	
	uint64_t o_entry_d;
	// Input for delivery
	uint64_t o_carrier_id;
	uint64_t ol_delivery_d;
	// for order-status


private:
	// warehouse id to partition id mapping
//	uint64_t wh_to_part(uint64_t wid);
	void gen_payment(uint64_t thd_id);
	void gen_new_order(uint64_t thd_id);
	void gen_order_status(uint64_t thd_id);
};

#endif
