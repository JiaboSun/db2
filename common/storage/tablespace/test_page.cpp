//
// Created by rrzhang on 2020/6/2.
//

#include <iostream>
#include <cstring>
#include "config.h"
#include "common/storage/tablespace/page.h"
#include "util/profiler.h"

using namespace std;

void Test_Page_Serialize() {
    /// Serialize
    dbx1000::Page *page1 = new dbx1000::Page(new char[MY_PAGE_SIZE]);
    page1->set_page_id(1);
    page1->set_page_size(MY_PAGE_SIZE);
    page1->set_used_size(64);
    page1->set_version(0);
    page1->Serialize();
    char buf[64];
    memcpy(buf, page1->page_buf(), 64);
    uint64_t page_id = *(reinterpret_cast<int64_t *>(&buf[0 * sizeof(uint64_t)]))
    , page_size = *(reinterpret_cast<int64_t *>(&buf[1 * sizeof(uint64_t)]))
    , used_size = *(reinterpret_cast<int64_t *>(&buf[2 * sizeof(uint64_t)]))
    , version = *(reinterpret_cast<int64_t *>(&buf[3 * sizeof(uint64_t)]));
    cout << page_id << ", " << page_size << ", " << used_size << ", " << version << endl;

    /// Deserialize
    dbx1000::Page *page2 = new dbx1000::Page(new char[MY_PAGE_SIZE]);
    page2->set_page_buf(page1->page_buf(), 64);
    page2->Deserialize();
    cout << page2->page_id() << ", " << page2->page_size() << ", " << page2->used_size() << ", " << page2->version() << endl;

    delete page1;
    delete page2;
}

//int main() {
//    Test_Serialize();
//    return 0;
//}