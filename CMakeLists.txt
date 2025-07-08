cmake_minimum_required(VERSION 3.22)

project(ucx_demo)

include(CTest)

set(CMAKE_CXX_STANDARD 23)

add_executable(ucx_demo
        src/main.cpp
        src/base.h src/base.cpp
        src/messaging.h src/messaging.cpp
)

find_package(UCX REQUIRED COMPONENTS ucp uct ucs)
target_link_libraries(ucx_demo ucx::ucp ucx::ucs)

find_package(CUDAToolkit REQUIRED)
target_link_libraries(ucx_demo CUDA::cudart)

add_test(
  NAME ${PROJECT_NAME}
  COMMAND
  sh -c
  "export UCX_LOG_LEVEL=INFO && ( $<TARGET_FILE:${PROJECT_NAME}> server 54321 & ( sleep 1 && $<TARGET_FILE:${PROJECT_NAME}> client 54321 ))")
