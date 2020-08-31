//
// Created by rrzhang on 2020/6/2.
//


#include <iostream>
#include <cstring>
#include "config.h"
#include "common/storage/tablespace/tablespace.h"
#include "util/profiler.h"

using namespace std;

void Test_TableSpace() {
    dbx1000::TableSpace *tableSpace = new dbx1000::TableSpace("test_table");
    tableSpace->set_table_size(12);
    tableSpace->set_page_size(13);
    tableSpace->set_last_page_id(14);
    tableSpace->Serialize();
    delete tableSpace;

    dbx1000::TableSpace *tableSpace2 = new dbx1000::TableSpace("test_table");
    tableSpace2->DeSerialize();
    cout << "table_name:" << tableSpace2->table_name() << ", table_size:" << tableSpace2->table_size()
         << ", row_size:" << tableSpace2->page_size() << ", last_page_id:" << tableSpace2->last_page_id() << endl;
    delete tableSpace2;
}