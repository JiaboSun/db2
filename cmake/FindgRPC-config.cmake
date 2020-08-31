### 尽量不要用，很坑

# Find Protobuf installation
# Looks for ProtobufConfig.cmake file installed by Protobuf's cmake installation.
set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG REQUIRED)
message(STATUS "Using protobuf ${Protobuf_VERSION}")
set(Protobuf_LIBRARY_SHARED protobuf::libprotobuf)
set(gRPC_GRPC++_REFLECTION_LIBRARY_SHARED gRPC::grpc++_reflection)
set(Protobuf_PROTOC_EXECUTABLE $<TARGET_FILE:protobuf::protoc>)

# Find gRPC installation
# Looks for gRPCConfig.cmake file installed by gRPC's cmake installation.
find_package(gRPC CONFIG REQUIRED)
message(STATUS "Using gRPC ${gRPC_VERSION}")
set(gRPC_GRPCPP_SHARED gRPC::grpc++)
set(gRPC_CPP_PLUGIN_EXECUTABLE $<TARGET_FILE:gRPC::grpc_cpp_plugin>)

message(STATUS "Protobuf_LIBRARY_SHARED              : ${Protobuf_LIBRARY_SHARED}")
message(STATUS "gRPC_GRPCPP_SHARED                   : ${gRPC_GRPCPP_SHARED}")
message(STATUS "gRPC_GRPC++_REFLECTION_LIBRARY_SHARED: ${gRPC_GRPC++_REFLECTION_LIBRARY_SHARED}")
message(STATUS "Protobuf_PROTOC_EXECUTABLE           : ${Protobuf_PROTOC_EXECUTABLE}")
message(STATUS "gRPC_CPP_PLUGIN_EXECUTABLE           : ${gRPC_CPP_PLUGIN_EXECUTABLE}")

set(Protobuf_INCLUDE_DIR ${Protobuf_ROOT}/include)
set(gRPC_INCLUDE_DIR     ${gRPC_ROOT}/include)

foreach (Camel
        Protobuf_LIBRARY_SHARED
        gRPC_GRPC++_REFLECTION_LIBRARY_SHARED
        Protobuf_PROTOC_EXECUTABLE
        gRPC_GRPCPP_SHARED
        gRPC_CPP_PLUGIN_EXECUTABLE
        )
    string(TOUPPER ${Camel} UPPER)
    set(${UPPER} ${${Camel}})
endforeach ()
