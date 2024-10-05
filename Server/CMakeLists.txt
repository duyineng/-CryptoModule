cmake_minimum_required(VERSION 3.0)
project(server)
set(CMAKE_CXX_STANDARD 17)

# 设置CMAKE_RUNTIME_OUTPUT_DIRECTORY变量，用来指定可执行文件的输出目录
# ${CMAKE_BINARY_DIR}指的是执行cmake ..命令时所在的目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/../share)

file(GLOB SERVER_SOURCES "*.cpp")
file(GLOB SHARED_SOURCES "${PROJECT_SOURCE_DIR}/../share/*.cpp")

add_executable(${PROJECT_NAME} ${SERVER_SOURCES} ${SHARED_SOURCES})