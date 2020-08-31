# find grpc, 测试对 grpc v1.23.x 有效，可以找到所有的静态库
message("researching for grpc v1.23.x")
set(GRPC_ROOT "${gRPC_ROOT}")
set(GRPC_FOUND 1)
set(gRPC_INCLUDE_DIR  ${GRPC_ROOT}/include)
set(gRPC_INCLUDE_DIRS ${GRPC_ROOT}/include)

# static lib
find_library(gRPC_ADDRESS_SORTING_LIBRARY      libaddress_sorting.a ${GRPC_ROOT}/lib NO_DEFAULT_PATH)
find_library(gRPC_GPR_LIBRARY                  libgpr.a ${GRPC_ROOT}/lib NO_DEFAULT_PATH)
find_library(gRPC_GRPC++_LIBRARY               libgrpc++.a ${GRPC_ROOT}/lib NO_DEFAULT_PATH)
find_library(gRPC_GRPCPP_LIBRARY               libgrpcpp.a ${GRPC_ROOT}/lib NO_DEFAULT_PATH)
find_library(gRPC_GRPC++_ERROR_DETAILS_LIBRARY libgrpc++_error_details.a ${GRPC_ROOT}/lib NO_DEFAULT_PATH)
find_library(gRPC_GRPCPP_ERROR_DETAILS_LIBRARY libgrpcpp_error_details.a ${GRPC_ROOT}/lib NO_DEFAULT_PATH)
find_library(gRPC_GRPC++_REFLECTION_LIBRARY    libgrpc++_reflection.a ${GRPC_ROOT}/lib NO_DEFAULT_PATH)
find_library(gRPC_GRPCPP_REFLECTION_LIBRARY    libgrpcpp_reflection.a ${GRPC_ROOT}/lib NO_DEFAULT_PATH)
find_library(gRPC_GRPC++_UNSECURE_LIBRARY      libgrpc++_unsecure.a ${GRPC_ROOT}/lib NO_DEFAULT_PATH)
find_library(gRPC_GRPCPP_UNSECURE_LIBRARY      libgrpcpp_unsecure.a ${GRPC_ROOT}/lib NO_DEFAULT_PATH)
find_library(gRPC_GRPC_LIBRARY                 libgrpc.a ${GRPC_ROOT}/lib NO_DEFAULT_PATH)
find_library(gRPC_GRPCPP_CHANNELZ_LIBRARY      libgrpcpp_channelz.a ${GRPC_ROOT}/lib NO_DEFAULT_PATH)
find_library(gRPC_GRPC_CRONET_LIBRARY          libgrpc_cronet.a ${GRPC_ROOT}/lib NO_DEFAULT_PATH)
find_library(gRPC_GRPC_UNSECURE_LIBRARY        libgrpc_unsecure.a ${GRPC_ROOT}/lib NO_DEFAULT_PATH)

set(GRPC_STATIC_LIBRARY
        ${gRPC_ADDRESS_SORTING_LIBRARY}
        ${gRPC_GPR_LIBRARY}
        ${gRPC_GRPC++_LIBRARY}
        ${gRPC_GRPCPP_LIBRARY}
        ${gRPC_GRPC++_ERROR_DETAILS_LIBRARY}
        ${gRPC_GRPCPP_ERROR_DETAILS_LIBRARY}
        ${gRPC_GRPC++_REFLECTION_LIBRARY}
        ${gRPC_GRPCPP_REFLECTION_LIBRARY}
        ${gRPC_GRPC++_UNSECURE_LIBRARY}
        ${gRPC_GRPCPP_UNSECURE_LIBRARY}
        ${gRPC_GRPC_LIBRARY}
        ${gRPC_GRPCPP_CHANNELZ_LIBRARY}
        ${gRPC_GRPC_CRONET_LIBRARY}
        ${gRPC_GRPC_UNSECURE_LIBRARY}
        )

message(STATUS "gRPC_GPR_LIBRARY                  : " ${gRPC_GPR_LIBRARY})
message(STATUS "gRPC_GRPC++_LIBRARY               : " ${gRPC_GRPC++_LIBRARY})
message(STATUS "gRPC_GRPCPP_LIBRARY               : " ${gRPC_GRPCPP_LIBRARY})
message(STATUS "gRPC_GRPC++_ERROR_DETAILS_LIBRARY : " ${gRPC_GRPC++_ERROR_DETAILS_LIBRARY})
message(STATUS "gRPC_GRPCPP_ERROR_DETAILS_LIBRARY : " ${gRPC_GRPCPP_ERROR_DETAILS_LIBRARY})
message(STATUS "gRPC_GRPC++_REFLECTION_LIBRARY    : " ${gRPC_GRPC++_REFLECTION_LIBRARY})
message(STATUS "gRPC_GRPCPP_REFLECTION_LIBRARY    : " ${gRPC_GRPCPP_REFLECTION_LIBRARY})
message(STATUS "gRPC_GRPC++_UNSECURE_LIBRARY      : " ${gRPC_GRPC++_UNSECURE_LIBRARY})
message(STATUS "gRPC_GRPCPP_UNSECURE_LIBRARY      : " ${gRPC_GRPCPP_UNSECURE_LIBRARY})
message(STATUS "gRPC_GRPC_LIBRARY                 : " ${gRPC_GRPC_LIBRARY})
message(STATUS "gRPC_GRPCPP_CHANNELZ_LIBRARY      : " ${gRPC_GRPCPP_CHANNELZ_LIBRARY})
message(STATUS "gRPC_GRPC_CRONET_LIBRARY          : " ${gRPC_GRPC_CRONET_LIBRARY})
message(STATUS "gRPC_GRPC_UNSECURE_LIBRARY        : " ${gRPC_GRPC_UNSECURE_LIBRARY})


# plugin
find_program(gRPC_CPP_PLUGIN_EXECUTABLE NAMES grpc_cpp_plugin PATHS ${GRPC_ROOT}/bin NO_DEFAULT_PATH)
message(STATUS "gRPC_CPP_PLUGIN_EXECUTABLE : " ${gRPC_CPP_PLUGIN_EXECUTABLE})

foreach (Camel
        gRPC_INCLUDE_DIR
        gRPC_ADDRESS_SORTING_LIBRARY
        gRPC_GPR_LIBRARY
        gRPC_GRPC++_LIBRARY
        gRPC_GRPCPP_LIBRARY
        gRPC_GRPC++_ERROR_DETAILS_LIBRARY
        gRPC_GRPCPP_ERROR_DETAILS_LIBRARY
        gRPC_GRPC++_REFLECTION_LIBRARY
        gRPC_GRPCPP_REFLECTION_LIBRARY
        gRPC_GRPC++_UNSECURE_LIBRARY
        gRPC_GRPCPP_UNSECURE_LIBRARY
        gRPC_GRPC_LIBRARY
        gRPC_GRPCPP_CHANNELZ_LIBRARY
        gRPC_GRPC_CRONET_LIBRARY
        gRPC_GRPC_UNSECURE_LIBRARY
        gRPC_CPP_PLUGIN_EXECUTABLE
        )
    string(TOUPPER ${Camel} UPPER)
    set(${UPPER} ${${Camel}})
endforeach ()