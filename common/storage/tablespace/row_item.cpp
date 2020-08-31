//
// Created by rrzhang on 2020/4/21.
//
#include <cstring>
#include "row_item.h"

#include "common/storage/table.h"

namespace dbx1000 {
    RowItem::RowItem(uint64_t key, size_t size)
            : key_(key)
              , size_(size) {
        if(0 != size_) {
            row_ = new char[size_];
        } else {
            row_ = nullptr;
        }
    }

	table_t * RowItem::get_table(){
		return table;
	}
              	
	Catalog * RowItem::get_schema(){
		return get_table()->get_schema();
	}
	void RowItem::set_value(int id, void * ptr) {
	int datasize = get_schema()->get_field_size(id);
	int pos = get_schema()->get_field_index(id);
	memcpy( &data[pos], ptr, datasize);
	}

	void RowItem::set_value(int id, void * ptr, int size) {
	int pos = get_schema()->get_field_index(id);
	memcpy( &data[pos], ptr, size);
	}

	void RowItem::set_value(const char * col_name, void * ptr) {
	uint64_t id = get_schema()->get_field_id(col_name);
	set_value(id, ptr);
	}

	char * RowItem::get_value(int id) {
		int pos = get_schema()->get_field_index(id);
		return &data[pos];
	}

	char * RowItem::get_value(char * col_name) {
		uint64_t pos = get_schema()->get_field_index(col_name);
		return &data[pos];
	}

	char * RowItem::get_data() 
	{
		return data; 
	}
    RowItem::~RowItem() {
        if(nullptr != row_) {
            delete row_;
        }
        size_ = 0;
        row_ = nullptr;
    }

//    void RowItem::init() {
//        row_ = new char[size_];
//    }
}