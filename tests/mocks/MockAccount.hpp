#pragma once
#include <gmock/gmock.h>
#include "Account.h"

// NiceMock подавляет предупреждения о "uninteresting mock function calls"
using ::testing::NiceMock;

class MockAccount : public Account {
public:
    MockAccount(int id, int balance) : Account(id, balance) {}
    
    MOCK_CONST_METHOD0(GetBalance, int());
    MOCK_METHOD1(ChangeBalance, void(int));
    MOCK_METHOD0(Lock, void());
    MOCK_METHOD0(Unlock, void());
};
