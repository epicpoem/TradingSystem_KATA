#include <iostream>
#include <gmock/gmock.h>

#include "AutoTradingSystem.cpp"

using namespace testing;

class TradingSystemFixture : public Test {
public:
	AutoTradingSystem tradingSystem;
};

TEST_F(TradingSystemFixture, TC1) {

}
