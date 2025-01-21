project(crypto_test)

file(GLOB UNIT_TEST_SRC "${PROJECT_ROOT_PATH}/tests/*_testing.cpp")
file(GLOB SRC
    "${PROJECT_ROOT_PATH}/../share/crypto/aes/*.cpp"
    "${PROJECT_ROOT_PATH}/../share/*.cpp"
)

add_executable(${PROJECT_NAME} ${UNIT_TEST_SRC} ${SRC})

target_include_directories(${PROJECT_NAME}
    PRIVATE
    "${PROJECT_ROOT_PATH}/../share/crypto/aes"
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    ${Protobuf_LIBRARIES} 
    gtest_main
    gmock_main
    OpenSSL::SSL
    OpenSSL::Crypto
)