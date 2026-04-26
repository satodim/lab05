cmake_minimum_required(VERSION 3.10)
project(banking)

option(BUILD_TESTS "Build tests" OFF)
option(COVERAGE "Enable code coverage" OFF)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(COVERAGE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -O0")
endif()

add_library(banking STATIC
    banking/Account.cpp
    banking/Transaction.cpp
)

target_include_directories(banking PUBLIC banking)

if(BUILD_TESTS)
    enable_testing()
    
    add_subdirectory(third-party/gtest)
    
    set(TEST_SOURCES
        tests/test_account.cpp
        tests/test_transaction.cpp
    )
    
    add_executable(check ${TEST_SOURCES})
    
    target_link_libraries(check banking gtest_main gmock)

    target_include_directories(check PRIVATE banking tests/mocks)
    
    if(COVERAGE)
        target_compile_options(check PRIVATE --coverage -O0)
        target_link_libraries(check --coverage)
    endif()
    
    add_test(NAME banking_tests COMMAND check)
endif()
