cmake_minimum_required(VERSION 3.22)

project(ucx_demo)

include(CTest)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/base.cpp
        src/messaging.cpp
)

find_package(UCX REQUIRED COMPONENTS ucp uct ucs)
target_link_libraries(${PROJECT_NAME} ucx::ucp ucx::ucs)

find_package(CUDAToolkit REQUIRED)
target_link_libraries(${PROJECT_NAME} CUDA::cudart)

add_test(
        NAME ${PROJECT_NAME}
        COMMAND
        sh -c
        "$<TARGET_FILE:${PROJECT_NAME}> server 54321 & ( sleep 1 && $<TARGET_FILE:${PROJECT_NAME}> client 54321 )")

set_tests_properties(
        ${PROJECT_NAME}
        PROPERTIES
        ENVIRONMENT
        "UCX_LOG_LEVEL=INFO"
)
