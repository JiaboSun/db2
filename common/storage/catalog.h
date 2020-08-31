#pragma once 

#include <map>
#include <vector>

//! 对列的描述，除了初始化，没有其他操作
class Column {
public:
	Column() {
	}
	Column(uint64_t size, std::string type, std::string name,
		uint64_t id, uint64_t index) 
	{
		this->size = size;
		this->id = id;
		this->index = index;
		this->type = type;
		this->name = name;
	};

	uint64_t    id;      //! 处在第几列
	uint32_t    size;    //! 该列的大小 ( in bytes )
	uint32_t    index;   //! 处在该行的第多少字节 ( in bytes )
	std::string type;    //! 类型
	std::string name;    //! 列名
//	char pad[CL_SIZE - sizeof(uint64_t)*3 - sizeof(char *)*2];
};

//! 相当于表空间，列的集合，函数都是对列的操作，不涉及行的操作
class Catalog {
public:
	// abandoned init function
	// field_size is the size of each each field.
	void init(std::string table_name, int field_cnt);
	void add_col(std::string col_name, uint64_t size,std::string type);

	uint32_t 		field_cnt;     //! 列数
 	std::string  	table_name;
	
	uint32_t 		get_tuple_size() { return tuple_size; };
	
	uint64_t 		get_field_cnt() { return field_cnt; };
	uint64_t 		get_field_size(int id) { return _columns[id].size; };
	uint64_t 		get_field_index(int id) { return _columns[id].index; };
	std::string  	get_field_type(uint64_t id);
	std::string		get_field_name(uint64_t id);
	uint64_t 		get_field_id(std::string  name);
	std::string 	get_field_type(std::string name);
	uint64_t 		get_field_index(std::string name);

	void 			print_schema();
	Column * 		_columns;       //! 列
	uint32_t 		tuple_size;     //! 一行的大小 (in bytes)
};

