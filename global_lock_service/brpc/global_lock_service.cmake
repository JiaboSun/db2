set(global_lock_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/global_lock_service.pb.cc")
set(global_lock_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/global_lock_service.pb.h")


### 最保险的做法，该命令会立即被调用。
execute_process(COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        -I ${CMAKE_SOURCE_DIR}/global_lock_service/proto
        --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/global_lock_service/proto/global_lock_service.proto)

include_directories(${PROJECT_SOURCE_DIR}/global_lock_service/brpc)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(GLOBAL_LOCK_SERVICE_FILES
        ${global_lock_proto_srcs}
        ${global_lock_proto_hdrs}
        global_lock_service/brpc/global_lock_service_helper.cpp
        global_lock_service/brpc/global_lock_service.cpp
        global_lock/global_lock.cpp)

add_library(global_lock_service SHARED ${GLOBAL_LOCK_SERVICE_FILES})
target_link_libraries(global_lock_service ${BRPC_LIB} ${DYNAMIC_LIB})
