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
        // Используем NiceMock для подавления предупреждений
        from = new NiceMock<MockAccount>(1, 500);
        to = new NiceMock<MockAccount>(2, 100);
        tx.set_fee(10);
    }
    
    void TearDown() override {
        delete from;
        delete to;
    }
    
    // Указатели на базовый класс MockAccount (не на NiceMock)
    MockAccount* from;
    MockAccount* to;
    Transaction tx;
};

// ============ Проверки исключений ============

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

// ============ Успешные операции ============

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

// ============ Ошибки и откаты ============

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

// ============ Базовые проверки (без моков) ============

TEST(TransactionUnitTest, DefaultFeeIsOne) {
    Transaction tx;
    EXPECT_EQ(tx.fee(), 1);
}

TEST(TransactionUnitTest, FeeCanBeChanged) {
    Transaction tx;
    tx.set_fee(42);
    EXPECT_EQ(tx.fee(), 42);
}
