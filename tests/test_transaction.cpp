#include <gtest/gtest.h>
#include "Transaction.h"
#include "Account.h"


TEST(TransactionTest, SameAccountThrows) {
    Account acc(1, 500);
    Transaction tx;
    EXPECT_THROW(tx.Make(acc, acc, 100), std::logic_error);
}

TEST(TransactionTest, NegativeSumThrows) {
    Account from(1, 500);
    Account to(2, 100);
    Transaction tx;
    EXPECT_THROW(tx.Make(from, to, -50), std::invalid_argument);
}

TEST(TransactionTest, SumTooSmallThrows) {
    Account from(1, 500);
    Account to(2, 100);
    Transaction tx;
    EXPECT_THROW(tx.Make(from, to, 50), std::logic_error);
}

TEST(TransactionTest, FeeTooHighReturnsFalse) {
    Account from(1, 500);
    Account to(2, 100);
    Transaction tx;
    tx.set_fee(200);
    
    EXPECT_FALSE(tx.Make(from, to, 300));
}

TEST(TransactionTest, DefaultFeeIsOne) {
    Transaction tx;
    EXPECT_EQ(tx.fee(), 1);
}

TEST(TransactionTest, SetFeeWorks) {
    Transaction tx;
    tx.set_fee(42);
    EXPECT_EQ(tx.fee(), 42);
}

TEST(TransactionTest, ValidTransferNoException) {
    Account from(1, 500);
    Account to(2, 100);
    Transaction tx;
    tx.set_fee(10);
    
    EXPECT_NO_THROW(tx.Make(from, to, 300));
}

TEST(TransactionTest, InsufficientFundsBehaviour) {
    Account from(1, 50); 
    Account to(2, 100);
    Transaction tx;
    tx.set_fee(10);
    
    EXPECT_NO_THROW(tx.Make(from, to, 100));
}

TEST(TransactionTest, ZeroFeeValidTransfer) {
    Account from(1, 500);
    Account to(2, 100);
    Transaction tx;
    tx.set_fee(0);
    
    EXPECT_NO_THROW(tx.Make(from, to, 300));
}

TEST(TransactionTest, MinimumValidSumNoException) {
    Account from(1, 500);
    Account to(2, 100);
    Transaction tx;
    tx.set_fee(1);
    
    EXPECT_NO_THROW(tx.Make(from, to, 100));
}

TEST(TransactionTest, LargeTransferNoException) {
    Account from(1, 1000);
    Account to(2, 100);
    Transaction tx;
    tx.set_fee(10);
    
    EXPECT_NO_THROW(tx.Make(from, to, 500));
}

TEST(TransactionTest, MultipleTransactionsSequential) {
    Account from(1, 1000);
    Account to(2, 100);
    Transaction tx;
    tx.set_fee(10);
    
    EXPECT_NO_THROW(tx.Make(from, to, 300));
    EXPECT_NO_THROW(tx.Make(from, to, 200));
}

TEST(TransactionTest, TransferDoesNotAffectFromBalance) {
    Account from(1, 500);
    Account to(2, 100);
    Transaction tx;
    tx.set_fee(10);
    
    int beforeBalance = from.GetBalance();
    tx.Make(from, to, 300);
    
    EXPECT_EQ(from.GetBalance(), beforeBalance);
}

TEST(TransactionTest, TransferAffectsToBalanceByFee) {
    Account from(1, 500);
    Account to(2, 100);
    Transaction tx;
    tx.set_fee(10);
    
    int beforeBalance = to.GetBalance();
    tx.Make(from, to, 300);
    
    EXPECT_LT(to.GetBalance(), beforeBalance);
}

TEST(TransactionTest, FeeIsAppliedToToAccount) {
    Account from(1, 500);
    Account to(2, 100);
    Transaction tx;
    tx.set_fee(15);
    
    int beforeBalance = to.GetBalance();
    tx.Make(from, to, 300);
    
    EXPECT_EQ(to.GetBalance(), beforeBalance - 15);
}
