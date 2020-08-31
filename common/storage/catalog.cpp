#include <cassert>
#include "catalog.h"

void Catalog::init(std::string table_name, int field_cnt) {
	this->table_name = table_name;
	this->field_cnt = 0;
	this->_columns = new Column [field_cnt];
	this->tuple_size = 0;
}

void Catalog::add_col(std::string col_name, uint64_t size, std::string type) {
	_columns[field_cnt].size = size;
	_columns[field_cnt].type = type;
	_columns[field_cnt].name = col_name;
	_columns[field_cnt].id = field_cnt;
	_columns[field_cnt].index = tuple_size;
	tuple_size += size;
	field_cnt ++;
}

uint64_t Catalog::get_field_id(std::string name) {
	uint32_t i;
	for (i = 0; i < field_cnt; i++) {
	    if(_columns[i].name.find(name) != std::string::npos)
//		if (strcmp(name, _columns[i].name) == 0)
			break;
	}
	assert (i < field_cnt);
	return i;
}

std::string Catalog::get_field_type(uint64_t id) {
	return _columns[id].type;
}

std::string Catalog::get_field_name(uint64_t id) {
	return _columns[id].name;
}


std::string Catalog::get_field_type(std::string name) {
	return get_field_type( get_field_id(name) );
}

uint64_t Catalog::get_field_index(std::string name) {
	return get_field_index( get_field_id(name) );
}

void Catalog::print_schema() {
	printf("\n[Catalog] %s\n", table_name.data());
	for (uint32_t i = 0; i < field_cnt; i++) {
		printf("\t%s\t%s\t%ld\n", get_field_name(i).data(),
			get_field_type(i).data(), get_field_size(i));
	}
}
