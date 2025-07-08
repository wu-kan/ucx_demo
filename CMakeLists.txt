cmake_minimum_required(VERSION 3.22)

project(ucx_demo)

include(CTest)

find_package(UCX REQUIRED COMPONENTS ucp uct ucs)

set(CMAKE_CXX_STANDARD 23)
add_executable(ucx_demo
        src/main.cpp
        src/base.h src/base.cpp
        src/messaging.h src/messaging.cpp
)

target_link_libraries(ucx_demo ucx::ucp ucx::ucs)

add_test(
  NAME ${PROJECT_NAME}
  COMMAND
  sh -c
  "export UCX_LOG_LEVEL=INFO && ( $<TARGET_FILE:${PROJECT_NAME}> server 54321 & ( sleep 1 && $<TARGET_FILE:${PROJECT_NAME}> client 54321 ))")
