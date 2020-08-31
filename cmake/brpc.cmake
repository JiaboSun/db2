### 指定依赖库的安装位置
list(APPEND CMAKE_INCLUDE_PATH "/home/zhangrongrong/.local/brpc-env/brpc/include")
list(APPEND CMAKE_LIBRARY_PATH "/home/zhangrongrong/.local/brpc-env/brpc/lib64")
list(APPEND CMAKE_INCLUDE_PATH "/home/zhangrongrong/.local/brpc-env/gflags/include")
list(APPEND CMAKE_LIBRARY_PATH "/home/zhangrongrong/.local/brpc-env/gflags/lib")
list(APPEND CMAKE_INCLUDE_PATH "/home/zhangrongrong/.local/brpc-env/leveldb/include")
list(APPEND CMAKE_LIBRARY_PATH "/home/zhangrongrong/.local/brpc-env/leveldb/lib")
list(APPEND CMAKE_INCLUDE_PATH "/home/zhangrongrong/.local/brpc-env/openssl-1.0.2k/include")
list(APPEND CMAKE_LIBRARY_PATH "/home/zhangrongrong/.local/brpc-env/openssl-1.0.2k/lib")
#list(APPEND CMAKE_INCLUDE_PATH "/home/zhangrongrong/.local/brpc-env/protobuf-4.0.x/include")
#list(APPEND CMAKE_LIBRARY_PATH "/home/zhangrongrong/.local/brpc-env/protobuf-4.0.x/lib")
set(Protobuf_ROOT "/home/zhangrongrong/.local/brpc-env/protobuf-4.0.x")

set(BRPC_INCLUDE_PATH "/home/zhangrongrong/.local/brpc-env/brpc/include")
set(BRPC_INCLUDE_DIR  ${BRPC_INCLUDE_PATH})
set(BRPC_INCLUDE_DIRS ${BRPC_INCLUDE_PATH})

### pthreads
include(FindThreads)

### Protobuf
find_package(Protobuf REQUIRED)
if ((NOT Protobuf_INCLUDE_DIRS) OR (NOT Protobuf_LIBRARIES))
    message(FATAL_ERROR "Fail to find brpc")
endif ()
#find_program(Protobuf_PROTOC_EXECUTABLE NAMES protoc PATHS /home/zhangrongrong/.local/brpc-env/protobuf-4.0.x/bin NO_DEFAULT_PATH)
message(STATUS "Using protobuf ${Protobuf_VERSION}")
message(STATUS "Protobuf_PROTOC_EXECUTABLE: ${Protobuf_PROTOC_EXECUTABLE}")
message(STATUS "Protobuf_INCLUDE_DIRS: ${Protobuf_INCLUDE_DIRS}")
message(STATUS "Protobuf_LIBRARIES: ${Protobuf_LIBRARIES}")
message(STATUS)
include_directories(${Protobuf_INCLUDE_DIR})


### BRPC
find_library(BRPC_LIB NAMES brpc)
if ((NOT BRPC_INCLUDE_PATH) OR (NOT BRPC_LIB))
    message(FATAL_ERROR "Fail to find brpc")
endif ()
message(STATUS "BRPC_INCLUDE_PATH: ${BRPC_INCLUDE_PATH}")
message(STATUS "BRPC_LIB: ${BRPC_LIB}")
message(STATUS)
include_directories(${BRPC_INCLUDE_PATH})

### GFLAGS
find_path(GFLAGS_INCLUDE_PATH gflags/gflags.h)
find_library(GFLAGS_LIBRARY NAMES gflags libgflags)
if ((NOT GFLAGS_INCLUDE_PATH) OR (NOT GFLAGS_LIBRARY))
    message(FATAL_ERROR "Fail to find gflags")
endif ()
execute_process(
        COMMAND bash -c "grep \"namespace [_A-Za-z0-9]\\+ {\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $2}' | tr -d '\n'"
        OUTPUT_VARIABLE GFLAGS_NS
)
if (${GFLAGS_NS} STREQUAL "GFLAGS_NAMESPACE")
    execute_process(
            COMMAND bash -c "grep \"#define GFLAGS_NAMESPACE [_A-Za-z0-9]\\+\" ${GFLAGS_INCLUDE_PATH}/gflags/gflags_declare.h | head -1 | awk '{print $3}' | tr -d '\n'"
            OUTPUT_VARIABLE GFLAGS_NS
    )
endif ()
set(CMAKE_CPP_FLAGS "${DEFINE_CLOCK_GETTIME} -DGFLAGS_NS=${GFLAGS_NS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CPP_FLAGS} -DNDEBUG -O2 -D__const__= -pipe -W -Wall -Wno-unused-parameter -fPIC -fno-omit-frame-pointer")
message(STATUS "GFLAGS_INCLUDE_DIRS: ${GFLAGS_INCLUDE_PATH}")
message(STATUS "GFLAGS_LIBRARIES: ${GFLAGS_LIBRARY}")
message(STATUS)
include_directories(${GFLAGS_INCLUDE_PATH})



### leveldb
find_path(LEVELDB_INCLUDE_PATH NAMES leveldb/db.h)
find_library(LEVELDB_LIB NAMES leveldb)
if ((NOT LEVELDB_INCLUDE_PATH) OR (NOT LEVELDB_LIB))
    message(FATAL_ERROR "Fail to find leveldb")
endif ()
message(STATUS "LEVELDB_INCLUDE_DIRS: ${LEVELDB_INCLUDE_PATH}")
message(STATUS "LEVELDB_LIBRARIES: ${LEVELDB_LIB}")
message(STATUS)
include_directories(${LEVELDB_INCLUDE_PATH})

### OpenSSL
#include_directories(${OPENSSL_INCLUDE_DIR})
find_package(OpenSSL REQUIRED)
message(STATUS "OPENSSL_VERSION: ${OPENSSL_VERSION}")
message(STATUS "OPENSSL_INCLUDE_DIR: ${OPENSSL_INCLUDE_DIR}")
message(STATUS "OPENSSL_LIBRARIES: ${OPENSSL_LIBRARIES}")
message(STATUS "OPENSSL_CRYPTO_LIBRARY: ${OPENSSL_CRYPTO_LIBRARY}")
message(STATUS)

if (WITH_TCMALLOC)
    find_library(TCMALLOC NAMES tcmalloc)
endif()
find_library(GZIP NAMES z)

set(DYNAMIC_LIB
    ${CMAKE_THREAD_LIBS_INIT}
    ${GFLAGS_LIBRARY}
    ${PROTOBUF_LIBRARIES}
    ${LEVELDB_LIB}
    ${OPENSSL_LIBRARIES}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${TCMALLOC}
    ${GZIP}
    dl
    )

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(DYNAMIC_LIB ${DYNAMIC_LIB}
        pthread
        "-framework CoreFoundation"
        "-framework CoreGraphics"
        "-framework CoreData"
        "-framework CoreText"
        "-framework Security"
        "-framework Foundation"
        "-Wl,-U,_MallocExtension_ReleaseFreeMemory"
        "-Wl,-U,_ProfilerStart"
        "-Wl,-U,_ProfilerStop"
        "-Wl,-U,_RegisterThriftProtocol")
endif()