cmake_minimum_required(VERSION 3.14)
project(Dbx1000)

SET(CMAKE_C_COMPILER "gcc")
SET(CMAKE_CXX_COMPILER "g++")
SET(CMAKE_CXX_FLAGS "-g3 -std=c++11 -Wno-deprecated-declarations" CACHE INTERNAL "compiler options" FORCE)
SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -g" CACHE INTERNAL "compiler options" FORCE)
SET(CMAKE_CXX_FLAGS_RELEASE "-O3" CACHE INTERNAL "compiler options" FORCE)

add_definitions(-DNOGRAPHITE=1)

### ******************************* grpc && protobuf *******************************:
#set(Protobuf_ROOT $ENV{HOME}/.local/protobuf-4.0.x)
#set(gRPC_ROOT     $ENV{HOME}/.local/grpc-v1.23.x)
#include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/protobuf-shared.cmake)
#include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/grpc-shared.cmake)
#find_library(ZLIB_LIBRARY libz.so /usr/lib/x86_64-linux-gnu/)
#include_directories(${Protobuf_INCLUDE_DIR})
#include_directories(${gRPC_INCLUDE_DIR})
### ******************************* grpc *******************************

include(cmake/brpc.cmake)

### include header files
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)


### lib files
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/libs)

set(COMMON_FILES
        common/index/index.cpp
        common/index/test_index.cpp
        common/lock_table/lock_table.cpp
        common/lock_table/test_lock_table.cpp
        common/storage/catalog.cpp
        common/storage/row_handler.cpp
        common/storage/table.cpp
        common/workload/ycsb_wl.cpp
        common/workload/wl.cpp
        common/global.cpp
        common/myhelper.cpp
        common/mystats.cpp
        common/parser.cpp
        common/parser_host.cpp
        util/arena.cpp
        util/parse_result.cpp
        config.cpp
        )
set(INSTANCE_FILES
        instance/benchmarks/ycsb_query.cpp
        instance/benchmarks/query.cpp
        instance/concurrency_control/row_mvcc.cpp
#        instance/concurrency_control/occ.cpp
#        instance/concurrency_control/row_occ.cpp
        instance/txn/ycsb_txn.cpp
        instance/txn/txn.cpp
        instance/manager_instance.cpp
        instance/thread.cpp
        )
set(GLOBAL_LOCK_FILES global_lock/global_lock.cpp)

#[[
编译模块：disk、tablespace、shared_disk、buffer、rpc_handler、lock_service
模块间的依赖关系：
        tablespace   -> libjsoncpp.a
        shared_disk  -> grpc,protobuf
        buffer       -> disk,tablespace,shared_disk
        rpc_handler 和 lock_service 本来是相互依赖的关系，但是后面编译时，会产生循环依赖，导致 rpc_handler 找不到 lock_service 的方法，
        所以在编译 rpc_handler 时，直接把 lock_service/manager_lock_service.cpp 作为库 rpc_handler 的源文件之一
        这样就只存在一个依赖：
        lock_service -> rpc_handler
]]
### disk
add_library(disk SHARED common/storage/disk/file_io.cpp common/storage/disk/memory_io.cpp)

### tablespace
add_library(tablespace SHARED common/storage/tablespace/page.cpp common/storage/tablespace/row_item.cpp common/storage/tablespace/tablespace.cpp)
target_link_libraries(tablespace libjsoncpp.a)

### shared_disk
include(shared_disk/brpc/shared_disk_service.cmake)

### buffer
file(GLOB buffer_src ${PROJECT_SOURCE_DIR}/common/buffer/*.cpp)
foreach(item ${buffer_src})
    if (${item} MATCHES "test*")
        list(REMOVE_ITEM buffer_src ${item})
    endif ()
endforeach()
add_library(buffer SHARED ${buffer_src})
target_link_libraries(buffer)

### global_lock_service
include(global_lock_service/brpc/global_lock_service.cmake)

### lock_service
add_library(global_lock ${GLOBAL_LOCK_FILES})
target_link_libraries(global_lock)



# main_lock_service
add_executable(main_lock_service test/main_lock_service.cpp ${INSTANCE_FILES} ${COMMON_FILES})
target_link_libraries(main_lock_service global_lock global_lock_service buffer shared_disk disk tablespace pthread tbb)
# main_instance
add_executable(main_instance test/main_instance.cpp ${INSTANCE_FILES} ${COMMON_FILES})
target_link_libraries(main_instance global_lock global_lock_service buffer shared_disk disk tablespace pthread tbb)
# disk_service
add_executable(disk_service test/disk_service.cpp)
target_link_libraries(disk_service shared_disk disk pthread tbb)
# gen_db
add_executable(gen_db test/gen_db.cpp ${INSTANCE_FILES} ${COMMON_FILES})
target_link_libraries(gen_db global_lock_service buffer shared_disk disk tablespace pthread tbb)

### ******************************* other test *******************************
# for buffer
add_executable(test_buffer common/buffer/test_buffer.cpp)
add_executable(test_lru common/buffer/test_lru.cpp)
add_executable(test_lru_index common/buffer/test_lru_index.cpp)
target_link_libraries(test_buffer    buffer  disk tablespace pthread tbb)
target_link_libraries(test_lru       buffer  disk tablespace pthread tbb)
target_link_libraries(test_lru_index buffer  disk tablespace pthread tbb)

add_executable(parse_result_test test/parse_result_test.cpp util/parse_result.cpp)

add_executable(other_test test/other_test.cpp)
target_link_libraries(other_test pthread tbb)
