cmake_minimum_required(VERSION 3.10)
project(banking)

option(BUILD_TESTS "Build tests" OFF)
set(CMAKE_CXX_STANDARD 11)

add_library(banking STATIC
    banking/Account.cpp
    banking/Transaction.cpp
)
target_include_directories(banking PUBLIC banking)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(third-party/gtest)
    
    add_executable(check 
        tests/test_account.cpp
        tests/test_transaction.cpp
    )
    
    target_link_libraries(check banking gtest_main gmock)
    target_include_directories(check PRIVATE banking tests/mocks)
    
    add_test(NAME banking_tests COMMAND check)
endif()
