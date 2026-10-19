cmake_minimum_required(VERSION 3.16)
project(trpc_homework LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(TRPC_ROOT_PATH "/opt/trpc" CACHE PATH "Built tRPC-Cpp source directory")
include(${TRPC_ROOT_PATH}/cmake/config/trpc_config.cmake)
include_directories(${INCLUDE_PATHS} ${CMAKE_CURRENT_BINARY_DIR})
link_directories(${LIBRARY_PATHS})
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/proto)
set(GENERATED)
foreach(NAME helloworld calc)
  set(BASE ${CMAKE_CURRENT_BINARY_DIR}/proto/${NAME})
  add_custom_command(
    OUTPUT ${BASE}.pb.cc ${BASE}.pb.h ${BASE}.trpc.pb.cc ${BASE}.trpc.pb.h
    COMMAND ${TRPC_ROOT_PATH}/build/bin/protoc
      --proto_path=${CMAKE_CURRENT_SOURCE_DIR}
      --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
      --plugin=protoc-gen-trpc=${TRPC_ROOT_PATH}/build/bin/trpc_cpp_plugin
      --trpc_out=${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_SOURCE_DIR}/proto/${NAME}.proto
    DEPENDS proto/${NAME}.proto VERBATIM)
  list(APPEND GENERATED ${BASE}.pb.cc ${BASE}.trpc.pb.cc)
endforeach()
add_library(rpc_interfaces STATIC ${GENERATED})
target_link_libraries(rpc_interfaces PUBLIC trpc ${LIBS_BASIC})
add_executable(rpc_server src/server_main.cc src/services.cc)
target_link_libraries(rpc_server PRIVATE rpc_interfaces)
add_executable(rpc_client src/client_main.cc)
target_link_libraries(rpc_client PRIVATE rpc_interfaces)
