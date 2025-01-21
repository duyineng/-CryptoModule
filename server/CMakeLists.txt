cmake_minimum_required(VERSION 3.12)    # VERSION是cmake_minimum_required命令的关键字，不是变量
project(server)             # 使得PROJECT_NAME的值变为server
set(CMAKE_CXX_STANDARD 17)  # 使用set命令设置CMAKE_CXX_STANDARD变量的值为17

find_package(Protobuf REQUIRED)
find_package(OpenSSL REQUIRED)

# 设置CMAKE_RUNTIME_OUTPUT_DIRECTORY变量的值，其值将指定add_executable命令生成的可执行文件的输出目录
# ${CMAKE_BINARY_DIR}指向的是执行cmake ..命令时所在的目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# PROJECT_ROOT_PATH为自定义的变量，将用来表示工程根目录，这是全局变量，在子CMakeLists.txt会继承
# ${PROJECT_SOURCE_DIR}指向最近一次调用project()命令的CMakeLists.txt文件的所在目录
set(PROJECT_ROOT_PATH ${PROJECT_SOURCE_DIR})

# GLOB表示使用通配符模式来匹配文件名
file(GLOB SERVER_SOURCES *.cpp)     # SERVER_SOURCES为局部变量，子CMakeLists.txt不会继承
file(GLOB_RECURSE SHARED_SOURCES 
    ${PROJECT_SOURCE_DIR}/../share/*.cpp
    ${PROJECT_SOURCE_DIR}/../share/*.cc
)

add_executable(${PROJECT_NAME} ${SERVER_SOURCES} ${SHARED_SOURCES})

# 如果不写LIST_DIRECTORIES true，将只收集文件
# 写上LIST_DIRECTORIES true，将会收集目录，比如${PROJECT_SOURCE_DIR}/../share会被收集
file(GLOB_RECURSE SHARE_DIRS LIST_DIRECTORIES true "${PROJECT_SOURCE_DIR}/../share/*")
set(SHARE_INCLUDE_DIRS "")  # 将SHARE_INCLUDE_DIRS变量的值设置为空列表
foreach(DIR ${SHARE_DIRS})
    if(IS_DIRECTORY ${DIR})
        list(APPEND SHARE_INCLUDE_DIRS ${DIR})
    endif()
endforeach()

# 这里的的包含目录，子CMakeLists.txt中的目标不会继承
# 子CMakeLists.txt中的目标需要单独设置它们的包含目录
target_include_directories(${PROJECT_NAME} 
    PRIVATE
    ${PROJECT_SOURCE_DIR}
    ${SHARE_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
    ${Protobuf_LIBRARIES} 
    OpenSSL::SSL
    OpenSSL::Crypto
)

add_subdirectory(third-part/googletest)
add_subdirectory(tests)
