set(sds_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/shared_disk_service.pb.cc")
set(sds_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/shared_disk_service.pb.h")


### 最保险的做法，该命令会立即被调用。
execute_process(COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        -I ${CMAKE_SOURCE_DIR}/shared_disk/proto
        --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/shared_disk/proto/shared_disk_service.proto)

include_directories(${PROJECT_SOURCE_DIR}/shared_disk/brpc)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SHARED_DISK_FILES
        ${sds_grpc_srcs}
        ${sds_proto_srcs}
        shared_disk/brpc/shared_disk_service.cpp)

add_library(shared_disk SHARED ${SHARED_DISK_FILES})
target_link_libraries(shared_disk ${BRPC_LIB} ${DYNAMIC_LIB})

