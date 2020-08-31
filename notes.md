几个主要模块：  
wl、query、txn、index、schame、concurrency_control(row_mvcc)、mem_alloc

wl 是负载模块，包含一个 index map、和一个 table map，ycsb_wl 是 wl 的一个派生类。  
该模块从文件中读取数据库的表的描述，并初始化内存中的索引、数据，ycsb 数据是 kv 键值对，key 从 0 自增，value 初始化成相同的值。  

query 负载生成查询队列，ycsb_request 结构体描述了查询类型（读、写、扫描）。

txn 各线程事务的基类，通过 run_txn 从 query 中取出一个请求执行。

row_mvcc 绑定到 row_t 结构体，每个row 都有一个 mvcc 对象管理当前 row 的并发访问。

mem_alloc，用于系统里批量结构体的内存管理，像 wl 中的索引，每个 kv 都要分配一个 Node 来管理；query 中的 ycsb_request 等；这些结构体需要大量使用，频繁使用系统的 new(或者 malloc) 函数容易造成内存碎片等，通过mem_alloc 先分配一个大的内存块，在给调用者使用。  
> 但是 dbx1000 里的这个 mem_alloc 有点问题，它默认最终还是使用的 malloc 来频繁向操作系统申请内存。后面我用 leveldb 里的 Arena 来替换了这个模块，但是使用时要注意并发访问的问题，因为 leveldb::Arena 并没有对多线程优化。

各模块的耦合度：

```
class wl{
private:	map<string, index_hash *> 	indexes;
			map<string, table_t *> 		tables;
}
table_t: 	Catalog 		*schema;

Catalog：	Column * 		_columns;

IndexHash:	
```
wl 包含一组 index 和 table_t 对象，table_t 通过 Catalog 来描述表的信息，Catalog 是一系列"列"的集合。  

IndexHash 包含了一个二维数组：BucketHeader _buckets[part_cnt][_bucket_cnt_per_part];   
part_cnt 是数据分区个数，_bucket_cnt_per_part 是每个分区内的数据量，满足 part_cnt * _bucket_cnt_per_part == 表的行数。  
BucketHeader 管理一组链表，节点由 BucketNode 描述。  
这么来说，假设行数为 1000，分区数 10，要存下这些数据，_buckets 完全够用，即 BucketHeader 链表长度始终为 1 即可，直接用 BucketNode _buckets[part_cnt][_bucket_cnt_per_part]; 不是更合适，为什么要用 链表？  
看 BucketHeader::insert_item() 代码，同一个 key 可能被多次写入，并不是直接覆盖之前的值，而是把同一个 key 的不同 row 用链表串起来，所以才用 BucketHeader。  
> 但是后面 txn 并没有多次写入同一个 key, 写动作只是记录在了 mvcc 里面，并没有更新索引。

```
Query_queue:	Query_thd ** all_queries;	// 

Query_thd:		ycsb_query * queries;

ycsb_query：	ycsb_request * requests;
```
Query_queue 是全局的查询队列，all_queries 数组大小和事务线程数量一致，每个事务有一个 Query_thd 队列。

ycsb_query 是一组 yscb 请求，每组请求有至多 16 个 ycsb_request。


wl、mvcc 放到远程，query、txn 放在各个计算机点。
	
	
	
	
	
	
## Manager	
全局的时间戳管理、锁管理等。

## storage
column、catalog、row、table之间的关系：  
column 是一个列的抽象包括在一行的第几个位置(id)，从一行的第多少字节开始(index),这一列占多少字节(size)，类型(type),名称(name)  
catalog 则是一个表的抽象，包括对所有列的描述(_columns),每行数据的长度(tuple_size),列数(field_cnt)  
table 包含了一个 catalog 指针，cur_tab_size表示该table内有所少行，还提供了 get_new_row(),来获取一个新的行。  
row_t 包含data，row id，分区 id，主键等。 

> TODO: row_t manager

## mem_alloc
内存区域的管理单元，系统使用一个全局的内存管理器 mem_alloc 为数据结构分配内存，避免频繁使用 malloc()。  
mem_alloc 含有一个 _arenas[n][4] 的数组(n表示使用该管理器的线程个数，4代表每个线程拥有 4 种 不同size的链表，分别为 32、64、256、1024)。  
每个 Arena 只是某个线程的某种size的链表。  
另外，mem_alloc 为每个线程创建了 <pthread_t, int> 这样的注册器，表示系统线程pid和index 的对应，index = 0...n-1, pid则由系统分配，  
这样线程调用 mem_alloc.alloc 时候就能通过调用者得 pid，来调用 _arenas[index]的分配器。  

struct free_block 链表的 node  
Arena 一个组织为链表结构的内存区域，使用时，Arena::alloc()先申请一块大内存，再按需分配给调用者。
mem_alloc，系统里的内存管理器，包含 Arena ** _arenas，以及 pid_arena 来记录线程pid和index的对应关系。

使用示例：  
ycsb_wl.cpp 里在加载数据的时候，调用 init_table_parallel() 根据 g_init_parallelism 去创建多个线程预热数据，单个区间(线程)的数据由 init_table_slice()加载，  
init_table_slice() 首先会调用 mem_allocator.register_thread(i) 去注册该线程，然后主线程 init_table_parallel() 在最后释放 unregister()。  
该内存分配器并不是为 row_t 分配的，而是分配给 itemid_t，itemid_t 有一个指针指向 row_t，row_t仍然是通过 malloc() 分配空间。  

#### 需要注意的是:
是否使用 Arena 来分配内存是由宏 THREAD_ALLOC 控制的，系统默认为 false，也就是说，默认还是用 malloc()，汗...  

#### 存在疑问:
既然是根据线程来并行，可是 mem_alloc 桶的数量 _bucket_cnt 设置成了 g_init_parallelism * 4 + 1，且和 init_thread_arena初始化的 _arenas 数量又不对应，这就很奇怪。

## query
ycsb_request 单个请求，记录查询的主键、类型、value(char)
ycsb_query : base_query	一个 query 包含多个请求：ycsb_request requests[16]。主要生成请求 gen_requests()，还有提供zipf分布的随机数。  
Query_thd	各线程，线程总数为全局 g_thread_cnt，包含一个查询数组: ycsb_query * queries;（该数组很大，约为100004）  
Query_queue	查询主线程，包含一个 Query_thd 队列：Query_thd ** all_queries，除了初始化和 get 下一个 query，没什么特别的。  


## wl, workload
该模块主要就是根据 txt里的表信息，来构建 schema（catalog），同时加载数据。  
基类两个成员：  
map<string, table_t *> tables;		用于存放表，string 为表名  
map<string, INDEX *> indexes;		存放索引，string 为表名  
tables 并不存数据，最后 row_t 只能通过 indexes 来获取。  



