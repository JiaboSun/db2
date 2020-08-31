//
// Created by rrzhang on 2020/4/21.
//

#ifndef DBX1000_ROW_ITEM_H
#define DBX1000_ROW_ITEM_H

#include <cstdint>
#include <cstdlib>

namespace dbx1000 {
    class RowItem {
    public:

        RowItem(uint64_t key, size_t size);
        RowItem() = delete;
        RowItem(const RowItem&) = delete;
        RowItem& operator=(const RowItem&) = delete;

		table_t * get_table();
		Catalog * get_schema();
		void set_value(int id, void * ptr);
		void set_value(int id, void * ptr, int size);
		void set_value(const char * col_name, void * ptr);
		char * get_value(int id);
		char * get_value(char * col_name);
		void set_data(char * data, uint64_t size);
		char * get_data();

        ~RowItem();

        uint64_t key_;
        size_t size_;
        char* row_;
		char * data;
		table_t * table;
    };
}

#endif //DBX1000_ROW_ITEM_H
