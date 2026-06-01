#include <gmock/gmock.h>
#include "AutoTradingSystem.cpp"

using namespace testing;

namespace {
	class MockStockerBrockerDriver : public StockerBrockerDriverInterface {
	public:
		MOCK_METHOD(void, login, (std::string ID, std::string password), (override));
		MOCK_METHOD(void, buy, (std::string stockCode, int count, int price), (override));
		MOCK_METHOD(void, sell, (std::string stockCode, int count, int price), (override));
		MOCK_METHOD(int, currentPrice, (std::string stockCode), (override));
	};

	class MockTimingStrategy : public ITimingStrategy {
	public:
		MOCK_METHOD(bool, shouldBuy, (const std::vector<int>& priceHistory), (override));
		MOCK_METHOD(bool, shouldSell, (const std::vector<int>& priceHistory), (override));
	};
}

// =============================================
// RisingTrendStrategy 테스트
// =============================================

class RisingTrendStrategyFixture : public Test {
public:
	RisingTrendStrategy strategy;
};

// [shouldBuy] 성공 케이스 - 3회 연속 상승 추세이면 true
TEST_F(RisingTrendStrategyFixture, shouldBuy_RisingTrend_ReturnsTrue) {
	EXPECT_THAT(strategy.shouldBuy({ 1000, 2000, 3000 }), Eq(true));
}

// [shouldBuy] 실패 케이스 - 하락 추세이면 false
TEST_F(RisingTrendStrategyFixture, shouldBuy_FallingTrend_ReturnsFalse) {
	EXPECT_THAT(strategy.shouldBuy({ 3000, 2000, 1000 }), Eq(false));
}

// [shouldBuy] 실패 케이스 - 횡보(동일 가격) 이면 false
TEST_F(RisingTrendStrategyFixture, shouldBuy_FlatPrices_ReturnsFalse) {
	EXPECT_THAT(strategy.shouldBuy({ 1000, 1000, 1000 }), Eq(false));
}

// [shouldBuy] 에러 케이스 - 가격 데이터가 3개 미만이면 false
// 상승 추세 판단 기준인 3개 가격이 부족하면 판단 불가 → false 반환
TEST_F(RisingTrendStrategyFixture, shouldBuy_Error_LessThan3Prices_ReturnsFalse) {
	EXPECT_THAT(strategy.shouldBuy({ 1000, 2000 }), Eq(false));
}

// [shouldBuy] 에러 케이스 - 빈 가격 이력이면 false
TEST_F(RisingTrendStrategyFixture, shouldBuy_Error_EmptyHistory_ReturnsFalse) {
	EXPECT_THAT(strategy.shouldBuy({}), Eq(false));
}

// [shouldSell] 성공 케이스 - 3회 연속 하락 추세이면 true
TEST_F(RisingTrendStrategyFixture, shouldSell_FallingTrend_ReturnsTrue) {
	EXPECT_THAT(strategy.shouldSell({ 3000, 2000, 1000 }), Eq(true));
}

// [shouldSell] 실패 케이스 - 상승 추세이면 false
TEST_F(RisingTrendStrategyFixture, shouldSell_RisingTrend_ReturnsFalse) {
	EXPECT_THAT(strategy.shouldSell({ 1000, 2000, 3000 }), Eq(false));
}

// [shouldSell] 에러 케이스 - 가격 데이터가 3개 미만이면 false
TEST_F(RisingTrendStrategyFixture, shouldSell_Error_LessThan3Prices_ReturnsFalse) {
	EXPECT_THAT(strategy.shouldSell({ 3000, 2000 }), Eq(false));
}

// =============================================
// MovingAverageStrategy 테스트 (기간: 3)
// =============================================

class MovingAverageStrategyFixture : public Test {
public:
	MovingAverageStrategy strategy{ 3 };
};

// [shouldBuy] 성공 케이스 - 현재가가 이동평균 초과이면 true
// 직전 3개 평균: (1000+2000+3000)/3 = 2000, 현재가 4000 > 2000 → true
TEST_F(MovingAverageStrategyFixture, shouldBuy_PriceAboveMA_ReturnsTrue) {
	EXPECT_THAT(strategy.shouldBuy({ 1000, 2000, 3000, 4000 }), Eq(true));
}

// [shouldBuy] 실패 케이스 - 현재가가 이동평균 미만이면 false
// 직전 3개 평균: (3000+2000+1000)/3 = 2000, 현재가 500 < 2000 → false
TEST_F(MovingAverageStrategyFixture, shouldBuy_PriceBelowMA_ReturnsFalse) {
	EXPECT_THAT(strategy.shouldBuy({ 3000, 2000, 1000, 500 }), Eq(false));
}

// [shouldBuy] 에러 케이스 - period(3)보다 데이터가 부족하면 false
// MA 계산에는 period+1개 이상의 가격이 필요하다 (MA 3개 + 현재가 1개 = 최소 4개)
TEST_F(MovingAverageStrategyFixture, shouldBuy_Error_InsufficientData_ReturnsFalse) {
	EXPECT_THAT(strategy.shouldBuy({ 1000, 2000, 3000 }), Eq(false));
}

// [shouldSell] 성공 케이스 - 현재가가 이동평균 미만이면 true
TEST_F(MovingAverageStrategyFixture, shouldSell_PriceBelowMA_ReturnsTrue) {
	EXPECT_THAT(strategy.shouldSell({ 3000, 2000, 1000, 500 }), Eq(true));
}

// [shouldSell] 실패 케이스 - 현재가가 이동평균 초과이면 false
TEST_F(MovingAverageStrategyFixture, shouldSell_PriceAboveMA_ReturnsFalse) {
	EXPECT_THAT(strategy.shouldSell({ 1000, 2000, 3000, 4000 }), Eq(false));
}

// [shouldSell] 에러 케이스 - period(3)보다 데이터가 부족하면 false
TEST_F(MovingAverageStrategyFixture, shouldSell_Error_InsufficientData_ReturnsFalse) {
	EXPECT_THAT(strategy.shouldSell({ 3000, 2000, 1000 }), Eq(false));
}

// =============================================
// BreakoutStrategy 테스트 (윈도우: 3)
// =============================================

class BreakoutStrategyFixture : public Test {
public:
	BreakoutStrategy strategy{ 3 };
};

// [shouldBuy] 성공 케이스 - 현재가가 직전 3개 고점을 돌파하면 true
// 직전 3개 고점: max(1000, 2000, 3000) = 3000, 현재가 4000 > 3000 → true
TEST_F(BreakoutStrategyFixture, shouldBuy_BreaksHighest_ReturnsTrue) {
	EXPECT_THAT(strategy.shouldBuy({ 1000, 2000, 3000, 4000 }), Eq(true));
}

// [shouldBuy] 실패 케이스 - 고점 미돌파이면 false
TEST_F(BreakoutStrategyFixture, shouldBuy_BelowHighest_ReturnsFalse) {
	EXPECT_THAT(strategy.shouldBuy({ 1000, 2000, 3000, 2500 }), Eq(false));
}

// [shouldBuy] 에러 케이스 - window(3)보다 데이터가 부족하면 false
// 직전 3개 고점 비교를 위해 window+1개 이상의 가격이 필요하다
TEST_F(BreakoutStrategyFixture, shouldBuy_Error_InsufficientData_ReturnsFalse) {
	EXPECT_THAT(strategy.shouldBuy({ 1000, 2000, 3000 }), Eq(false));
}

// [shouldSell] 성공 케이스 - 현재가가 직전 3개 저점을 하향 돌파하면 true
// 직전 3개 저점: min(3000, 2000, 1000) = 1000, 현재가 500 < 1000 → true
TEST_F(BreakoutStrategyFixture, shouldSell_BreaksLowest_ReturnsTrue) {
	EXPECT_THAT(strategy.shouldSell({ 3000, 2000, 1000, 500 }), Eq(true));
}

// [shouldSell] 실패 케이스 - 저점 미돌파이면 false
TEST_F(BreakoutStrategyFixture, shouldSell_AboveLowest_ReturnsFalse) {
	EXPECT_THAT(strategy.shouldSell({ 3000, 2000, 1000, 1500 }), Eq(false));
}

// [shouldSell] 에러 케이스 - window(3)보다 데이터가 부족하면 false
TEST_F(BreakoutStrategyFixture, shouldSell_Error_InsufficientData_ReturnsFalse) {
	EXPECT_THAT(strategy.shouldSell({ 3000, 2000, 1000 }), Eq(false));
}

// =============================================
// AutoTradingSystem + ITimingStrategy DI 테스트
// =============================================

class AutoTradingSystemStrategyFixture : public Test {
public:
	NiceMock<MockStockerBrockerDriver> mockDriver;
	NiceMock<MockTimingStrategy> mockStrategy;
	AutoTradingSystem tradingSystem;

	void SetUp() override {
		tradingSystem.selectStockBrocker(&mockDriver);
		tradingSystem.setStrategy(&mockStrategy);
	}
};

// [자동매수] 성공 케이스 - 전략이 매수 승인(true)이면 buy() 호출됨
// 3회 가격 수집 후 shouldBuy()가 true → 마지막 가격으로 최대 수량 매수
TEST_F(AutoTradingSystemStrategyFixture, buyNiceTiming_StrategyApproves_BuyIsCalled) {
	EXPECT_CALL(mockDriver, currentPrice("005930"))
		.WillOnce(Return(1000))
		.WillOnce(Return(2000))
		.WillOnce(Return(3000));
	EXPECT_CALL(mockStrategy, shouldBuy(_)).WillOnce(Return(true));
	EXPECT_CALL(mockDriver, buy("005930", 3, 3000)).Times(1);

	tradingSystem.buyNiceTiming("005930", 9000);
}

// [자동매수] 실패 케이스 - 전략이 매수 거부(false)이면 buy() 미호출
// shouldBuy()가 false이면 가격 추세와 무관하게 매수하지 않는다.
TEST_F(AutoTradingSystemStrategyFixture, buyNiceTiming_StrategyDenies_BuyIsNotCalled) {
	EXPECT_CALL(mockDriver, currentPrice("005930"))
		.WillOnce(Return(3000))
		.WillOnce(Return(2000))
		.WillOnce(Return(1000));
	EXPECT_CALL(mockStrategy, shouldBuy(_)).WillOnce(Return(false));
	EXPECT_CALL(mockDriver, buy(_, _, _)).Times(0);

	tradingSystem.buyNiceTiming("005930", 9000);
}

// [자동매도] 성공 케이스 - 전략이 매도 승인(true)이면 sell() 호출됨
// 3회 가격 수집 후 shouldSell()이 true → 마지막 가격으로 지정 수량 전량 매도
TEST_F(AutoTradingSystemStrategyFixture, sellNiceTiming_StrategyApproves_SellIsCalled) {
	EXPECT_CALL(mockDriver, currentPrice("005930"))
		.WillOnce(Return(3000))
		.WillOnce(Return(2000))
		.WillOnce(Return(1000));
	EXPECT_CALL(mockStrategy, shouldSell(_)).WillOnce(Return(true));
	EXPECT_CALL(mockDriver, sell("005930", 10, 1000)).Times(1);

	tradingSystem.sellNiceTiming("005930", 10);
}

// [자동매도] 실패 케이스 - 전략이 매도 거부(false)이면 sell() 미호출
TEST_F(AutoTradingSystemStrategyFixture, sellNiceTiming_StrategyDenies_SellIsNotCalled) {
	EXPECT_CALL(mockDriver, currentPrice("005930"))
		.WillOnce(Return(1000))
		.WillOnce(Return(2000))
		.WillOnce(Return(3000));
	EXPECT_CALL(mockStrategy, shouldSell(_)).WillOnce(Return(false));
	EXPECT_CALL(mockDriver, sell(_, _, _)).Times(0);

	tradingSystem.sellNiceTiming("005930", 10);
}

// [자동매수] 에러 케이스 - 전략 미설정 시 false 반환
// setStrategy()를 호출하지 않은 상태에서 buyNiceTiming() 시도 시 false를 반환한다.
TEST(AutoTradingSystemNoStrategyTest, buyNiceTiming_NoStrategySet_ReturnsFalse) {
	AutoTradingSystem system;
	NiceMock<MockStockerBrockerDriver> mockDriver;
	system.selectStockBrocker(&mockDriver);

	EXPECT_THAT(system.buyNiceTiming("005930", 9000), Eq(false));
}

// [자동매도] 에러 케이스 - 전략 미설정 시 false 반환
TEST(AutoTradingSystemNoStrategyTest, sellNiceTiming_NoStrategySet_ReturnsFalse) {
	AutoTradingSystem system;
	NiceMock<MockStockerBrockerDriver> mockDriver;
	system.selectStockBrocker(&mockDriver);

	EXPECT_THAT(system.sellNiceTiming("005930", 10), Eq(false));
}
