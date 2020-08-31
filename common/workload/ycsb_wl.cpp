

#include <sched.h>
#include <thread>
#include <cstring>

#include "ycsb_wl.h"

#include "common/global.h"
#include "common/buffer/buffer.h"
#include "common/index/index.h"
#include "common/storage/disk/file_io.h"
#include "common/storage/tablespace/page.h"
#include "common/storage/tablespace/row_item.h"
#include "common/storage/tablespace/tablespace.h"
#include "common/storage/catalog.h"
#include "common/storage/table.h"
#include "util/profiler.h"
#include "util/arena.h"

std::atomic<int> ycsb_wl::next_tid;

ycsb_wl::ycsb_wl(){
    cout << "ycsb_wl::ycsb_wl()" << endl;
}

ycsb_wl::~ycsb_wl(){
    cout << "ycsb_wl::~ycsb_wl()" << endl;
}

RC ycsb_wl::init() {
    cout << "ycsb_wl::init()" << endl;
	workload::init();
	next_tid = 0;
    init_schema(g_schame_path);

//	init_table();
	return RC::RCOK;
}

RC ycsb_wl::init_schema(string schema_file) {
	workload::init_schema(schema_file);
	the_table = tables["MAIN_TABLE"];
//	the_index = indexes["MAIN_INDEX"];
	return RC::RCOK;
}
//! 数据是分区的，每个区间的 key 都是 0-g_synth_table_size / g_part_cnt，单调增，返回 key 在哪个区间
int
ycsb_wl::key_to_part(uint64_t key) {
	uint64_t rows_per_part = g_synth_table_size / g_part_cnt;
	return key / rows_per_part;
}

RC ycsb_wl::init_table() {
    std::unique_ptr<dbx1000::Profiler> profiler(new dbx1000::Profiler());
    profiler->Start();

    dbx1000::TableSpace *tableSpace = new dbx1000::TableSpace(the_table->get_table_name());
    dbx1000::Index *index = new dbx1000::Index(the_table->get_table_name() + "_INDEX");
    uint32_t tuple_size = the_table->get_schema()->get_tuple_size();
    tableSpace->set_row_size(tuple_size);

    dbx1000::Page *page = new dbx1000::Page(new char[MY_PAGE_SIZE]);
    page->set_page_id(tableSpace->GetNextPageId());
    char row[tuple_size];
    uint64_t version = 0;
    memset(row, 'a', tuple_size);
    for (uint64_t key = 0; key < g_synth_table_size; key++) {
        if (tuple_size > (MY_PAGE_SIZE - page->used_size())) {
            page->Serialize();
            dbx1000::FileIO::WritePage(page->page_id(), page->page_buf());
            assert((((MY_PAGE_SIZE - 64) / tuple_size * tuple_size) + 64) ==
                   page->used_size());  /// 检查 used_size
            page->set_page_id(tableSpace->GetNextPageId());
            page->set_used_size(64);
        }
        memcpy(&row[0], &key, sizeof(uint64_t));    /// 头 8 字节为 key
        memcpy(&row[tuple_size - 8], &version, sizeof(uint64_t));   /// 尾 8 字节为 version
        page->PagePut(page->page_id(), row, tuple_size);
        dbx1000::IndexItem indexItem(page->page_id(), page->used_size() - tuple_size);
        index->IndexPut(key, &indexItem);
    }
    if (page->used_size() > 64) {
        page->Serialize();
        dbx1000::FileIO::WritePage(page->page_id(), page->page_buf());
    }
    index->Serialize();
    tableSpace->Serialize();
    delete page;
    delete index;
    delete tableSpace;

//    init_table_parallel();
    profiler->End();
    std::cout << "ycsb_wl::init_table, workload Init Time : " << profiler->Millis() << " Millis" << std::endl;
    return RC::RCOK;
}

// init table in parallel
void ycsb_wl::init_table_parallel() {

    std::vector<std::thread> v_thread;
    for(int i = 0; i < g_init_parallelism; i++) {
        v_thread.emplace_back(thread(threadInitTable, this));
    }
    for(int i = 0; i < g_init_parallelism; i++) {
        v_thread[i].join();
    }
}
//! 初始化单个区间
void * ycsb_wl::init_table_slice() {
    uint32_t tid = next_tid.fetch_add(1);

//	cout << tid << endl;
//	set_affinity(tid);      /// 绑定到物理核

    RC rc;
    assert(g_synth_table_size % g_init_parallelism == 0);
    assert(tid < g_init_parallelism);
    uint64_t slice_size = g_synth_table_size / g_init_parallelism;

    uint32_t tuple_size = the_table->get_schema()->get_tuple_size();

    for (uint64_t key = slice_size * tid; key < slice_size * (tid + 1); key++) {

    }
}