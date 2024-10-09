cmake_minimum_required(VERSION 3.12)
project(server)
set(CMAKE_CXX_STANDARD 17)

find_package(Protobuf REQUIRED)

# 设置CMAKE_RUNTIME_OUTPUT_DIRECTORY变量，用来指定可执行文件的输出目录
# ${CMAKE_BINARY_DIR}指的是执行cmake ..命令时所在的目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/../share)
include_directories(${PROJECT_SOURCE_DIR}/../share/codec)
include_directories(${Protobuf_INCLUDE_DIRS})

file(GLOB SERVER_SOURCES *.cpp)
file(GLOB SHARED_SOURCES ${PROJECT_SOURCE_DIR}/../share/*.cpp ${PROJECT_SOURCE_DIR}/../share/codec/*.cpp ${PROJECT_SOURCE_DIR}/../share/codec/*.cc)
add_executable(${PROJECT_NAME} ${SERVER_SOURCES} ${SHARED_SOURCES})
target_link_libraries(${PROJECT_NAME} ${Protobuf_LIBRARIES})