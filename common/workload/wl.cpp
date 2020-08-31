#include <fstream>
#include "wl.h"

#include "common/global.h"
#include "common/storage/catalog.h"
#include "common/storage/table.h"
#include "common/buffer/buffer.h"
#include "util/arena.h"
#include "util/numbercomparator.h"
#include "util/make_unique.h"

workload::workload(){
    cout << "workload::workload()" << endl;
}
workload::~workload(){
    cout << "workload::~workload()" << endl;
    for(int i = 0; i < g_init_parallelism; i++) {
        delete arenas_[i];
    }
}

RC workload::init() {
    cout << "workload::init()" << endl;
    for (int i = 0; i < g_init_parallelism; i++) {
        arenas_.emplace_back(new dbx1000::Arena(i));
    }

    sim_done_ = false;
    return RC::RCOK;
}

RC workload::init_schema(string schema_file) {
    assert(sizeof(uint64_t) == 8);
    assert(sizeof(double) == 8);
    string line;
    ifstream fin(schema_file);
    Catalog *schema = new Catalog();
    /// while 循环读取行，不包括每行尾的 '\n'
    while (getline(fin, line)) {
        /// such as TABLE=MAIN_TABLE
        if (line.compare(0, 6, "TABLE=") == 0) {
			std::string tname = std::string(&line[6], line.size() - 6);
            getline(fin, line);
            int col_count = 0;
            // Read all fields for this table.
            vector<string> lines;
            /// 该 while 读取直到遇到空行
            while (line.length() > 1) {
                lines.push_back(line);
                getline(fin, line);
            }
            schema->init(tname, lines.size());
            for (uint32_t i = 0; i < lines.size(); i++) {
                string line = lines[i];
                size_t pos = 0;
                string token;
                int elem_num = 0;
                int size = 0;
                string type;
                string name;
                while (line.length() != 0) {
                    pos = line.find(",");
                    if (pos == string::npos)
                        pos = line.length();
                    token = line.substr(0, pos);
                    line.erase(0, pos + 1);
                    switch (elem_num) {
                        case 0:
                            size = atoi(token.c_str());
                            break;
                        case 1:
                            type = token;
                            break;
                        case 2:
                            name = token;
                            break;
                        default:
                            assert(false);
                    }
                    elem_num++;
                }
                assert(elem_num == 3);
                schema->add_col(name, size, type);
                col_count++;
            }
            table_t *cur_tab = new table_t();
            cur_tab->init(schema);
            tables[tname] = cur_tab;
        }
//        else if (!line.compare(0, 6, "INDEX=")) {
//            string iname;
//            iname = &line[6];
//            getline(fin, line);
//
//            vector<string> items;
//            string token;
//            size_t pos;
//            while (line.length() != 0) {
//                pos = line.find(",");
//                if (pos == string::npos)
//                    pos = line.length();
//                token = line.substr(0, pos);
//                items.push_back(token);
//                line.erase(0, pos + 1);
//            }
//
//            string tname(items[0]);
//            INDEX *index = new INDEX();
//            int part_cnt = (CENTRAL_INDEX) ? 1 : g_part_cnt;
//            if (tname == "ITEM")
//                part_cnt = 1;
//#if INDEX_STRUCT == IDX_HASH
//#if WORKLOAD == YCSB
//            index->init(part_cnt, tables[tname], g_synth_table_size * 2);
//#elif WORKLOAD == TPCC
//            assert(tables[tname] != NULL);
//            index->init(part_cnt, tables[tname], stoi( items[1] ) * part_cnt);
//#endif
//#else
//            index->init(part_cnt, tables[tname]);
//#endif
//            indexes[iname] = index;
//        }
    }
    fin.close();
    return RC::RCOK;
}