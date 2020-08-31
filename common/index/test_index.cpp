#include <iostream>
#include "index.h"
#include "json/json.h"
#include "config.h"
#include "util/profiler.h"
#include <sstream>

using namespace std;
#define INDEX_NUM (1024 * 1024 *10)

void Test_Index() {
    dbx1000::Profiler profiler;

    //////////////////////////////////////////////
    dbx1000::Index *index2 = new dbx1000::Index("test_index");
    for (int i = 0; i < INDEX_NUM; i++) {
        dbx1000::IndexItem *indexItem = new dbx1000::IndexItem(i, i);
        index2->IndexPut(i, indexItem);
    }
    profiler.Clear();
    profiler.Start();
    index2->Serialize();
    profiler.End();
    cout << "to file1 time : " << profiler.Millis() << endl;

    //////////////////////////////////////////////
    dbx1000::Index *index3 = new dbx1000::Index("test_index");
    profiler.Clear();
    profiler.Start();
    index3->DeSerialize();
    profiler.End();
    cout << "from file1 time : " << profiler.Millis() << endl;
//    index3->Print();


//////////////////////////////////////////////

    dbx1000::Index *index4 = new dbx1000::Index("test_index");
    for (int i = 0; i < INDEX_NUM; i++) {
        dbx1000::IndexItem *indexItem = new dbx1000::IndexItem(i, i);
        index4->IndexPut(i, indexItem);
    }
    profiler.Clear();
    profiler.Start();
    index4->Serialize2();
    profiler.End();
    cout << "to file2 time : " << profiler.Millis() << endl;

    dbx1000::Index *index5 = new dbx1000::Index("test_index");
    profiler.Clear();
    profiler.Start();
    index5->DeSerialize2();
    profiler.End();
    cout << "from file2 time : " << profiler.Millis() << endl;
//    index5->Print();


    delete index2;
    delete index3;
    delete index4;
    delete index5;
}