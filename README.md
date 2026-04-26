#Homework
## 1 Клонируем репозиторий
```sh
export GITHUB_USERNAME=satodim
cd ${GITHUB_USERNAME}/workspace/workspace/projects
git clone https://github.com/${GITHUB_USERNAME}/lab04 lab05
cd lab05
git remote remove origin
git remote add origin https://github.com/${GITHUB_USERNAME}/lab05
```
## 2 Добавляем Gtest
```sh
mkdir third-party
git submodule add https://github.com/google/googletest third-party/gtest
cd third-party/gtest && git checkout release-1.8.1 && cd ../..
```
## 3 Создадим данный нам репозиторий banking и создадим в нем 4 данных нам файла
*сделали это с помощью nano*
*Account.h*
```sh
#pragma once
class Account {
 public:
  Account(int id, int balance);
  virtual ~Account();

  // Virtual to test.
  virtual int GetBalance() const;

  // Virtual to test.
  virtual void ChangeBalance(int diff);

  // Virtual to test.
  virtual void Lock();

  // Virtual to test.
  virtual void Unlock();
  int id() const { return id_; }

 private:
  int id_;
  int balance_;
  bool is_locked_;
};
```
*Account.cpp*
```sh
#include "Account.h"

#include <stdexcept>

Account::Account(int id, int balance)
    : id_(id), balance_(balance), is_locked_(false) {}

Account::~Account() {}

int Account::GetBalance() const { return balance_; }

void Account::ChangeBalance(int diff) {
  if (!is_locked_) throw std::runtime_error("at first lock the account");
  balance_ += diff;
}

void Account::Lock() {
  if (is_locked_) throw std::runtime_error("already locked");
  is_locked_ = true;
}

void Account::Unlock() { is_locked_ = false; }
```
*Transaction.h*
```sh
#pragma once

class Account;

class Transaction {
 public:
  Transaction();
  virtual ~Transaction();

  bool Make(Account& from, Account& to, int sum);
  int fee() const { return fee_; }
  void set_fee(int fee) { fee_ = fee; }

 private:
  void Credit(Account& accout, int sum);
  bool Debit(Account& accout, int sum);

  // Virtual to test.
  virtual void SaveToDataBase(Account& from, Account& to, int sum);

  int fee_;
};
```
*Transaction.cpp*
```sh
#include "Transaction.h"

#include <cassert>
#include <iostream>
#include <stdexcept>

#include "Account.h"

namespace {
// RAII
struct Guard {
  Guard(Account& account) : account_(&account) { account_->Lock(); }

  ~Guard() { account_->Unlock(); }

 private:
  Account* account_;
};
}  // namespace

Transaction::Transaction() : fee_(1) {}

Transaction::~Transaction() {}

bool Transaction::Make(Account& from, Account& to, int sum) {
  if (from.id() == to.id()) throw std::logic_error("invalid action");

  if (sum < 0) throw std::invalid_argument("sum can't be negative");

  if (sum < 100) throw std::logic_error("too small");

  if (fee_ * 2 > sum) return false;

  Guard guard_from(from);
  Guard guard_to(to);

  Credit(to, sum);

  bool success = Debit(to, sum + fee_);
  if (!success) to.ChangeBalance(-sum);

  SaveToDataBase(from, to, sum);
  return success;
}

void Transaction::Credit(Account& accout, int sum) {
  assert(sum > 0);
  accout.ChangeBalance(sum);
}

bool Transaction::Debit(Account& accout, int sum) {
  assert(sum > 0);
  if (accout.GetBalance() > sum) {
    accout.ChangeBalance(-sum);
    return true;
  }
  return false;
}

void Transaction::SaveToDataBase(Account& from, Account& to, int sum) {
  std::cout << from.id() << " send to " << to.id() << " $" << sum << std::endl;
  std::cout << "Balance " << from.id() << " is " << from.GetBalance()
            << std::endl;
  std::cout << "Balance " << to.id() << " is " << to.GetBalance() << std::endl;
}
```
## 5 Создадим директорию tests, где напишем тестовые файлы
```sh
mkdir tests
```
*И теперь напишем с помощью nano файлы test_transaction.cpp и test_account.cpp*
*<test_account.cpp>*
```sh
#include <gtest/gtest.h>
#include "Account.h"

TEST(AccountTest, ConstructorAndGetters) {
    Account acc(1, 100);
    EXPECT_EQ(acc.id(), 1);
    EXPECT_EQ(acc.GetBalance(), 100);
}

TEST(AccountTest, ChangeBalanceRequiresLock) {
    Account acc(1, 100);
    
    EXPECT_THROW(acc.ChangeBalance(50), std::runtime_error);
    
    acc.Lock();
    EXPECT_NO_THROW(acc.ChangeBalance(50));
    EXPECT_EQ(acc.GetBalance(), 150);
}

TEST(AccountTest, DoubleLockThrows) {
    Account acc(1, 100);
    acc.Lock();
    EXPECT_THROW(acc.Lock(), std::runtime_error);
}

TEST(AccountTest, UnlockAllowsChangeBalance) {
    Account acc(1, 100);
    acc.Lock();
    acc.Unlock();
    EXPECT_THROW(acc.ChangeBalance(50), std::runtime_error);
}

TEST(AccountTest, MultipleLockUnlock) {
    Account acc(1, 100);
    
    acc.Lock();
    acc.ChangeBalance(100);
    acc.Unlock();
    EXPECT_THROW(acc.ChangeBalance(50), std::runtime_error);
    
    acc.Lock();
    EXPECT_NO_THROW(acc.ChangeBalance(50));
    EXPECT_EQ(acc.GetBalance(), 250);
}
```
*<test_transaction.cpp>*
```sh
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "Transaction.h"
#include "mocks/MockAccount.hpp"

using ::testing::Return;
using ::testing::Throw;
using ::testing::NiceMock;

class TransactionTest : public ::testing::Test {
protected:
    void SetUp() override {
        from = new NiceMock<MockAccount>(1, 500);
        to = new NiceMock<MockAccount>(2, 100);
        tx.set_fee(10);
    }
    
    void TearDown() override {
        delete from;
        delete to;
    }
    
    MockAccount* from;
    MockAccount* to;
    Transaction tx;
};


TEST_F(TransactionTest, ThrowsOnSameAccount) {
    EXPECT_THROW(tx.Make(*from, *from, 100), std::logic_error);
}

TEST_F(TransactionTest, ThrowsOnNegativeSum) {
    EXPECT_THROW(tx.Make(*from, *to, -50), std::invalid_argument);
}

TEST_F(TransactionTest, ThrowsOnSumTooSmall) {
    EXPECT_THROW(tx.Make(*from, *to, 50), std::logic_error);
}

TEST_F(TransactionTest, ReturnsFalseWhenFeeTooHigh) {
    tx.set_fee(200);
    EXPECT_FALSE(tx.Make(*from, *to, 300));
}


TEST_F(TransactionTest, ExecutesSuccessfully) {
    EXPECT_CALL(*from, Lock());
    EXPECT_CALL(*to, Lock());
    EXPECT_CALL(*to, ChangeBalance(300));
    EXPECT_CALL(*from, GetBalance()).WillRepeatedly(Return(500));
    EXPECT_CALL(*from, ChangeBalance(-310));
    
    EXPECT_TRUE(tx.Make(*from, *to, 300));
}

TEST_F(TransactionTest, WorksWithZeroFee) {
    tx.set_fee(0);
    
    EXPECT_CALL(*from, Lock());
    EXPECT_CALL(*to, Lock());
    EXPECT_CALL(*to, ChangeBalance(300));
    EXPECT_CALL(*from, GetBalance()).WillRepeatedly(Return(500));
    EXPECT_CALL(*from, ChangeBalance(-300));
    
    EXPECT_TRUE(tx.Make(*from, *to, 300));
}

TEST_F(TransactionTest, WorksWithMinimumSum) {
    tx.set_fee(1);
    
    EXPECT_CALL(*from, Lock());
    EXPECT_CALL(*to, Lock());
    EXPECT_CALL(*to, ChangeBalance(100));
    EXPECT_CALL(*from, GetBalance()).WillRepeatedly(Return(500));
    EXPECT_CALL(*from, ChangeBalance(-101));
    
    EXPECT_TRUE(tx.Make(*from, *to, 100));
}

TEST_F(TransactionTest, ReturnsFalseWhenInsufficientFunds) {
    EXPECT_CALL(*from, Lock());
    EXPECT_CALL(*to, Lock());
    EXPECT_CALL(*to, ChangeBalance(200));
    EXPECT_CALL(*from, GetBalance()).WillRepeatedly(Return(50));
    EXPECT_CALL(*to, ChangeBalance(-200));  // откат
    
    EXPECT_FALSE(tx.Make(*from, *to, 200));
}

TEST_F(TransactionTest, PropagatesLockException) {
    EXPECT_CALL(*from, Lock());
    EXPECT_CALL(*to, Lock()).WillOnce(Throw(std::runtime_error("already locked")));
    
    EXPECT_THROW(tx.Make(*from, *to, 300), std::runtime_error);
}


TEST(TransactionUnitTest, DefaultFeeIsOne) {
    Transaction tx;
    EXPECT_EQ(tx.fee(), 1);
}

TEST(TransactionUnitTest, FeeCanBeChanged) {
    Transaction tx;
    tx.set_fee(42);
    EXPECT_EQ(tx.fee(), 42);
}
```
## 6 Создадим директорию mocks, где напишем MockAccount.hpp для того, где переопределим виртуальные методы Account
```sh
mkdir mocks
```
*<MockAccount.hpp>*
```sh
#pragma once
#include <gmock/gmock.h>
#include "Account.h"

using ::testing::NiceMock;

class MockAccount : public Account {
public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    
    MOCK_CONST_METHOD0(GetBalance, int());
    MOCK_METHOD1(ChangeBalance, void(int));
    MOCK_METHOD0(Lock, void());
    MOCK_METHOD0(Unlock, void());
};
```
## 7 Теперь напишем общий CMakeLists.txt ,чтобы окончательно собрать проект.
```sh
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
```
## 8 Настраиваем linux.yml для Github Actions
```sh
name: CMake Build and Test with Coverage

on:
  push:
    branches: [ main, master ]
  pull_request:
    branches: [ main, master ]

jobs:
  build:
    runs-on: ubuntu-latest

    steps:
    - uses: actions/checkout@v4
      with:
        submodules: recursive

    - name: Configure CMake
      run: |
        cmake -H. -B_build \
          -DBUILD_TESTS=ON \
          -DCMAKE_CXX_FLAGS="--coverage -O0 -Wno-error -Wno-maybe-uninitialized" \
          -DCMAKE_EXE_LINKER_FLAGS="--coverage"

    - name: Build
      run: cmake --build _build

    - name: Run tests
      working-directory: _build
      run: ctest --output-on-failure

    - name: Generate coverage with gcov
      working-directory: _build
      run: |
        for file in $(find . -name "*.gcda" | grep banking); do
          gcov -o $(dirname $file) $(basename $file .gcda).gcda 2>/dev/null || true
        done

        echo "=== Coverage Report ==="
        for f in *.cpp.gcov; do
          if [ -f "$f" ]; then
            echo ""
            echo "--- $f ---"
            grep "Lines executed" "$f" || echo "No coverage data"
          fi
        done

    - name: Upload coverage artifacts
      uses: actions/upload-artifact@v4
      with:
        name: coverage-reports
        path: _build/*.gcov
```
## 9 Теперь соберем проект и запустимм тесты
```sh
cmake -H. -B_build \
  -DBUILD_TESTS=ON \
  -DCMAKE_CXX_FLAGS="--coverage -O0 -Wno-error -Wno-maybe-uninitialized" \
  -DCMAKE_EXE_LINKER_FLAGS="--coverage"
cmake --build _build
_build/check
```
*Вывод1:*
```sh
-- The C compiler identification is GNU 13.3.0
-- The CXX compiler identification is GNU 13.3.0
-- Detecting C compiler ABI info
-- Detecting C compiler ABI info - done
-- Check for working C compiler: /usr/bin/cc - skipped
-- Detecting C compile features
-- Detecting C compile features - done
-- Detecting CXX compiler ABI info
-- Detecting CXX compiler ABI info - done
-- Check for working CXX compiler: /usr/bin/c++ - skipped
-- Detecting CXX compile features
-- Detecting CXX compile features - done
CMake Deprecation Warning at third-party/gtest/CMakeLists.txt:1 (cmake_minimum_required):
  Compatibility with CMake < 3.5 will be removed from a future version of
  CMake.

  Update the VERSION argument <min> value or use a ...<max> suffix to tell
  CMake that the project does not need compatibility with older versions.


CMake Deprecation Warning at third-party/gtest/googlemock/CMakeLists.txt:42 (cmake_minimum_required):
  Compatibility with CMake < 3.5 will be removed from a future version of
  CMake.

  Update the VERSION argument <min> value or use a ...<max> suffix to tell
  CMake that the project does not need compatibility with older versions.


CMake Deprecation Warning at third-party/gtest/googletest/CMakeLists.txt:49 (cmake_minimum_required):
  Compatibility with CMake < 3.5 will be removed from a future version of
  CMake.

  Update the VERSION argument <min> value or use a ...<max> suffix to tell
  CMake that the project does not need compatibility with older versions.


CMake Warning (dev) at third-party/gtest/googletest/cmake/internal_utils.cmake:239 (find_package):
  Policy CMP0148 is not set: The FindPythonInterp and FindPythonLibs modules
  are removed.  Run "cmake --help-policy CMP0148" for policy details.  Use
  the cmake_policy command to set the policy and suppress this warning.

Call Stack (most recent call first):
  third-party/gtest/googletest/CMakeLists.txt:84 (include)
This warning is for project developers.  Use -Wno-dev to suppress it.

-- Found PythonInterp: /usr/bin/python3 (found version "3.12.3") 
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD
-- Performing Test CMAKE_HAVE_LIBC_PTHREAD - Success
-- Found Threads: TRUE  
-- Configuring done (1.5s)
-- Generating done (0.0s)
-- Build files have been written to: /home/vboxuser/satodim/workspace/workspace/projects/lab05/_build
```
*Предупреждения возникают из-за того, что используется старая версия Google Test, как в tutorial, но это не мешает работе*

*Вывод2:*
```sh
[  7%] Building CXX object CMakeFiles/banking.dir/banking/Account.cpp.o
[ 14%] Building CXX object CMakeFiles/banking.dir/banking/Transaction.cpp.o
[ 21%] Linking CXX static library libbanking.a
[ 21%] Built target banking
[ 28%] Building CXX object third-party/gtest/googlemock/gtest/CMakeFiles/gtest.dir/src/gtest-all.cc.o
[ 35%] Linking CXX static library libgtest.a
[ 35%] Built target gtest
[ 42%] Building CXX object third-party/gtest/googlemock/CMakeFiles/gmock.dir/src/gmock-all.cc.o
[ 50%] Linking CXX static library libgmock.a
[ 50%] Built target gmock
[ 57%] Building CXX object third-party/gtest/googlemock/gtest/CMakeFiles/gtest_main.dir/src/gtest_main.cc.o
[ 64%] Linking CXX static library libgtest_main.a
[ 64%] Built target gtest_main
[ 71%] Building CXX object CMakeFiles/check.dir/tests/test_account.cpp.o
[ 78%] Building CXX object CMakeFiles/check.dir/tests/test_transaction.cpp.o
[ 85%] Linking CXX executable check
[ 85%] Built target check
[ 92%] Building CXX object third-party/gtest/googlemock/CMakeFiles/gmock_main.dir/src/gmock_main.cc.o
[100%] Linking CXX static library libgmock_main.a
[100%] Built target gmock_main
```
*Вывод3:*
```sh
Running main() from /home/vboxuser/satodim/workspace/workspace/projects/lab05/third-party/gtest/googletest/src/gtest_main.cc
[==========] Running 16 tests from 3 test cases.
[----------] Global test environment set-up.
[----------] 5 tests from AccountTest
[ RUN      ] AccountTest.ConstructorAndGetters
[       OK ] AccountTest.ConstructorAndGetters (0 ms)
[ RUN      ] AccountTest.ChangeBalanceRequiresLock
[       OK ] AccountTest.ChangeBalanceRequiresLock (54 ms)
[ RUN      ] AccountTest.DoubleLockThrows
[       OK ] AccountTest.DoubleLockThrows (0 ms)
[ RUN      ] AccountTest.UnlockAllowsChangeBalance
[       OK ] AccountTest.UnlockAllowsChangeBalance (0 ms)
[ RUN      ] AccountTest.MultipleLockUnlock
[       OK ] AccountTest.MultipleLockUnlock (0 ms)
[----------] 5 tests from AccountTest (55 ms total)

[----------] 9 tests from TransactionTest
[ RUN      ] TransactionTest.ThrowsOnSameAccount
[       OK ] TransactionTest.ThrowsOnSameAccount (0 ms)
[ RUN      ] TransactionTest.ThrowsOnNegativeSum
[       OK ] TransactionTest.ThrowsOnNegativeSum (0 ms)
[ RUN      ] TransactionTest.ThrowsOnSumTooSmall
[       OK ] TransactionTest.ThrowsOnSumTooSmall (0 ms)
[ RUN      ] TransactionTest.ReturnsFalseWhenFeeTooHigh
[       OK ] TransactionTest.ReturnsFalseWhenFeeTooHigh (0 ms)
[ RUN      ] TransactionTest.ExecutesSuccessfully
1 send to 2 $300
Balance 1 is 500
Balance 2 is 0
[       OK ] TransactionTest.ExecutesSuccessfully (0 ms)
[ RUN      ] TransactionTest.WorksWithZeroFee
1 send to 2 $300
Balance 1 is 500
Balance 2 is 0
[       OK ] TransactionTest.WorksWithZeroFee (0 ms)
[ RUN      ] TransactionTest.WorksWithMinimumSum
1 send to 2 $100
Balance 1 is 500
Balance 2 is 0
[       OK ] TransactionTest.WorksWithMinimumSum (0 ms)
[ RUN      ] TransactionTest.ReturnsFalseWhenInsufficientFunds
1 send to 2 $200
Balance 1 is 50
Balance 2 is 0
[       OK ] TransactionTest.ReturnsFalseWhenInsufficientFunds (0 ms)
[ RUN      ] TransactionTest.PropagatesLockException
[       OK ] TransactionTest.PropagatesLockException (0 ms)
[----------] 9 tests from TransactionTest (1 ms total)

[----------] 2 tests from TransactionUnitTest
[ RUN      ] TransactionUnitTest.DefaultFeeIsOne
[       OK ] TransactionUnitTest.DefaultFeeIsOne (0 ms)
[ RUN      ] TransactionUnitTest.FeeCanBeChanged
[       OK ] TransactionUnitTest.FeeCanBeChanged (0 ms)
[----------] 2 tests from TransactionUnitTest (0 ms total)

[----------] Global test environment tear-down
[==========] 16 tests from 3 test cases ran. (57 ms total)
[  PASSED  ] 16 tests.

```
## Все выполнилось, осталось проверить процент покрытия с помощью утилиты gcov
```sh
cd _build
for file in $(find . -name "*.gcda" | grep banking); do
    gcov -o $(dirname $file) $(basename $file .gcda).gcda 2>/dev/null
done
```
*Вывод:*
```sh
File '/home/vboxuser/satodim/workspace/workspace/projects/lab05/banking/Transaction.cpp'
Lines executed:97.14% of 35
Creating 'Transaction.cpp.gcov'

File '/home/vboxuser/satodim/workspace/workspace/projects/lab05/banking/Account.h'
Lines executed:0.00% of 1
Creating 'Account.h.gcov'

File '/home/vboxuser/satodim/workspace/workspace/projects/lab05/banking/Account.cpp'
Lines executed:93.33% of 15
Creating 'Account.cpp.gcov'
```
*Покрытие для Account.h 0 % - это заголовочный файл, так и должно быть,просто gcov нашел одну исполняемую строчку*
```sh
int id() const { return id_; }
```
