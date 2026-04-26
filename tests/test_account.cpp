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
