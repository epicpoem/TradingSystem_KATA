#include <iostream>
#include <gmock/gmock.h>

#include "AutoTradingSystem.cpp"

using namespace testing;

class MockStockerBrockerDriver : public StockerBrockerDriverInterface {
public:
	MOCK_METHOD(void, login, (std::string ID, std::string password), (override));
	MOCK_METHOD(void, buy, (std::string stockCode, int count, int price), (override));
	MOCK_METHOD(void, sell, (std::string stockCode, int count, int price), (override));
	MOCK_METHOD(int, currentPrice, (std::string stockCode), (override));
};

class TradingSystemFixture : public Test {
public:
	NiceMock<MockStockerBrockerDriver> mockDriver;
	RisingTrendStrategy strategy;
	AutoTradingSystem tradingSystem;

	void SetUp() override {
		tradingSystem.selectStockBrocker(&mockDriver);
		tradingSystem.setStrategy(&strategy);
	}
};

// [로그인] 성공 케이스
// 유효한 ID/PW로 login() 호출 시 드라이버의 login()이 정확히 1회 호출된다.
TEST_F(TradingSystemFixture, Login_Success) {
	EXPECT_CALL(mockDriver, login("testID", "testPass")).Times(1);
	tradingSystem.login("testID", "testPass");
}

// [로그인] 실패 케이스 - 증권사 미선택
// selectStockBrocker()를 호출하지 않은 상태에서 login() 시도 시 false를 반환한다.
TEST_F(TradingSystemFixture, Login_Fail_NoBrokerSelected) {
	AutoTradingSystem systemWithoutBroker;
	EXPECT_THAT(systemWithoutBroker.login("testID", "testPass"), Eq(false));
}

// [로그인] 에러 케이스 - 빈 ID
// ID에 빈 문자열("")을 전달하면 입력값이 유효하지 않으므로 false를 반환한다.
// 빈 ID는 서버 요청 전 클라이언트 단에서 사전 차단해야 한다.
TEST_F(TradingSystemFixture, Login_Error_EmptyID) {
	EXPECT_THAT(tradingSystem.login("", "testPass"), Eq(false));
}

// [로그인] 에러 케이스 - 빈 패스워드
// Password에 빈 문자열("")을 전달하면 입력값이 유효하지 않으므로 false를 반환한다.
// 빈 Password는 서버 요청 전 클라이언트 단에서 사전 차단해야 한다.
TEST_F(TradingSystemFixture, Login_Error_EmptyPassword) {
	EXPECT_THAT(tradingSystem.login("testID", ""), Eq(false));
}

// [매수] 성공 케이스
// buy(종목코드, 가격, 수량) 호출 시 드라이버의 buy(종목코드, 수량, 가격)이 정확히 1회 호출된다.
// AutoTradingSystem API와 드라이버 인터페이스 간 (가격, 수량) 파라미터 순서가 역전됨에 주의.
TEST_F(TradingSystemFixture, buy_Success) {
	EXPECT_CALL(mockDriver, buy("005930", 10, 50000)).Times(1);
	tradingSystem.buy("005930", 50000, 10);
}

// [매수] 실패 케이스 - 증권사 미선택
// selectStockBrocker()를 호출하지 않은 상태에서 buy() 시도 시 false를 반환한다.
TEST_F(TradingSystemFixture, buy_Fail_NoBrokerSelected) {
	AutoTradingSystem systemWithoutBroker;
	EXPECT_THAT(systemWithoutBroker.buy("005930", 50000, 10), Eq(false));
}

// [매수] 에러 케이스 - 빈 종목코드
// 종목코드에 빈 문자열("")을 전달하면 조회할 종목을 특정할 수 없으므로 false를 반환한다.
// 빈 종목코드로 드라이버를 호출할 경우 예측 불가한 동작이 발생할 수 있어 사전 차단이 필요하다.
TEST_F(TradingSystemFixture, buy_Error_InvalidStockCode) {
	EXPECT_THAT(tradingSystem.buy("", 50000, 10), Eq(false));
}

// [매수] 에러 케이스 - 유효하지 않은 가격 (0원)
// 가격이 0이면 실제 체결이 불가능한 값이므로 false를 반환한다.
// 0원 또는 음수 가격은 거래소 규정상 허용되지 않으며, 드라이버 호출 전에 차단해야 한다.
TEST_F(TradingSystemFixture, buy_Error_InvalidPrice) {
	EXPECT_THAT(tradingSystem.buy("005930", 0, 10), Eq(false));
}

// [매수] 에러 케이스 - 유효하지 않은 수량 (0주)
// 수량이 0이면 매수할 주식이 없으므로 false를 반환한다.
// 0주 또는 음수 수량은 의미 없는 요청으로, 드라이버 호출 전에 차단해야 한다.
TEST_F(TradingSystemFixture, buy_Error_InvalidAmount) {
	EXPECT_THAT(tradingSystem.buy("005930", 50000, 0), Eq(false));
}

// [매도] 성공 케이스
// sell(종목코드, 가격, 수량) 호출 시 드라이버의 sell(종목코드, 수량, 가격)이 정확히 1회 호출된다.
// AutoTradingSystem API와 드라이버 인터페이스 간 (가격, 수량) 파라미터 순서가 역전됨에 주의.
TEST_F(TradingSystemFixture, sell_Success) {
	EXPECT_CALL(mockDriver, sell("005930", 10, 55000)).Times(1);
	tradingSystem.sell("005930", 55000, 10);
}

// [매도] 실패 케이스 - 증권사 미선택
// selectStockBrocker()를 호출하지 않은 상태에서 sell() 시도 시 false를 반환한다.
TEST_F(TradingSystemFixture, sell_Fail_NoBrokerSelected) {
	AutoTradingSystem systemWithoutBroker;
	EXPECT_THAT(systemWithoutBroker.sell("005930", 55000, 10), Eq(false));
}

// [매도] 에러 케이스 - 빈 종목코드
// 종목코드에 빈 문자열("")을 전달하면 매도할 종목을 특정할 수 없으므로 false를 반환한다.
// 빈 종목코드로 드라이버를 호출할 경우 예측 불가한 동작이 발생할 수 있어 사전 차단이 필요하다.
TEST_F(TradingSystemFixture, sell_Error_InvalidStockCode) {
	EXPECT_THAT(tradingSystem.sell("", 55000, 10), Eq(false));
}

// [매도] 에러 케이스 - 유효하지 않은 가격 (0원)
// 가격이 0이면 실제 체결이 불가능한 값이므로 false를 반환한다.
// 0원 또는 음수 가격은 거래소 규정상 허용되지 않으며, 드라이버 호출 전에 차단해야 한다.
TEST_F(TradingSystemFixture, sell_Error_InvalidPrice) {
	EXPECT_THAT(tradingSystem.sell("005930", 0, 10), Eq(false));
}

// [매도] 에러 케이스 - 유효하지 않은 수량 (0주)
// 수량이 0이면 매도할 주식이 없으므로 false를 반환한다.
// 0주 또는 음수 수량은 의미 없는 요청으로, 드라이버 호출 전에 차단해야 한다.
TEST_F(TradingSystemFixture, sell_Error_InvalidAmount) {
	EXPECT_THAT(tradingSystem.sell("005930", 55000, 0), Eq(false));
}

// [현재가] 성공 케이스
// 유효한 종목코드로 getPrice() 호출 시 드라이버가 반환한 현재가를 그대로 반환한다.
TEST_F(TradingSystemFixture, getPrice_Success) {
	EXPECT_CALL(mockDriver, currentPrice("005930")).WillOnce(Return(50000));
	EXPECT_THAT(tradingSystem.getPrice("005930"), Eq(50000));
}

// [현재가] 실패 케이스 - 증권사 미선택
// selectStockBrocker()를 호출하지 않은 상태에서 getPrice() 시도 시 -1을 반환한다.
TEST_F(TradingSystemFixture, getPrice_Fail_NoBrokerSelected) {
	AutoTradingSystem systemWithoutBroker;
	EXPECT_THAT(systemWithoutBroker.getPrice("005930"), Eq(-1));
}

// [현재가] 에러 케이스 - 빈 종목코드
// 종목코드에 빈 문자열("")을 전달하면 조회할 종목을 특정할 수 없으므로 -1을 반환한다.
// 빈 종목코드로 드라이버를 호출하면 의미 없는 네트워크 요청이 발생하므로 사전 차단이 필요하다.
TEST_F(TradingSystemFixture, getPrice_Error_EmptyStockCode) {
	EXPECT_THAT(tradingSystem.getPrice(""), Eq(-1));
}

// [현재가] 에러 케이스 - 드라이버가 0을 반환 (유효하지 않은 시세)
// 드라이버가 현재가로 0을 반환하면 시세 데이터가 없거나 조회 실패로 간주하여 -1을 반환한다.
// 실제 주식 가격은 0원이 될 수 없으므로 0은 오류 상황으로 처리해야 한다.
TEST_F(TradingSystemFixture, getPrice_Error_ZeroPrice) {
	EXPECT_CALL(mockDriver, currentPrice("000000")).WillOnce(Return(0));
	EXPECT_THAT(tradingSystem.getPrice("000000"), Eq(-1));
}

// [자동매수] 성공 케이스
// 200ms 간격으로 3회 가격 조회 시 상승 추세(p1 < p2 < p3)이면
// 총금액을 마지막 가격으로 나눈 최대 수량만큼 마지막 가격에 매수한다.
// 예) 가격: 1000 → 2000 → 3000, 총금액: 9000 → 수량: 9000/3000=3, 가격: 3000
TEST_F(TradingSystemFixture, buyNiceTiming_Success) {
	EXPECT_CALL(mockDriver, currentPrice("005930"))
		.WillOnce(Return(1000))
		.WillOnce(Return(2000))
		.WillOnce(Return(3000));
	EXPECT_CALL(mockDriver, buy("005930", 3, 3000)).Times(1);

	tradingSystem.buyNiceTiming("005930", 9000);
}

// [자동매수] 실패 케이스 - 상승추세 아님
// 200ms 간격으로 3회 가격 조회 시 하락 추세(p1 > p2 > p3)이면 매수를 수행하지 않는다.
// 가격: 3000 → 2000 → 1000 (하락추세) → buy() 호출 없음
TEST_F(TradingSystemFixture, buyNiceTiming_Fail_NotRisingTrend) {
	EXPECT_CALL(mockDriver, currentPrice("005930"))
		.WillOnce(Return(3000))
		.WillOnce(Return(2000))
		.WillOnce(Return(1000));
	EXPECT_CALL(mockDriver, buy(_, _, _)).Times(0);

	tradingSystem.buyNiceTiming("005930", 9000);
}

// [자동매수] 에러 케이스 - 빈 종목코드
// 종목코드에 빈 문자열("")을 전달하면 가격 조회 자체가 불가능하므로 즉시 false를 반환한다.
// currentPrice() 호출 없이 입력 검증 단계에서 차단되어야 불필요한 드라이버 접근을 막을 수 있다.
TEST_F(TradingSystemFixture, buyNiceTiming_Error_InvalidStockCode) {
	EXPECT_THAT(tradingSystem.buyNiceTiming("", 9000), Eq(false));
}

// [자동매수] 에러 케이스 - 총금액이 0
// 총금액이 0이면 매수할 수 있는 수량이 0주이므로 즉시 false를 반환한다.
// 0원으로는 어떤 종목도 매수할 수 없으며, 가격 조회 없이 입력 검증 단계에서 차단되어야 한다.
TEST_F(TradingSystemFixture, buyNiceTiming_Error_ZeroTotalAmount) {
	EXPECT_THAT(tradingSystem.buyNiceTiming("005930", 0), Eq(false));
}

// [자동매수] 에러 케이스 - 총금액이 음수
// 총금액이 음수이면 논리적으로 유효하지 않은 금액이므로 즉시 false를 반환한다.
// 음수 금액은 매수 의도가 없는 잘못된 입력으로, 입력 검증 단계에서 차단되어야 한다.
TEST_F(TradingSystemFixture, buyNiceTiming_Error_NegativeTotalAmount) {
	EXPECT_THAT(tradingSystem.buyNiceTiming("005930", -9000), Eq(false));
}

// [자동매도] 성공 케이스
// 200ms 간격으로 3회 가격 조회 시 하락 추세(p1 > p2 > p3)이면
// 지정한 수량 전체를 마지막 가격에 매도한다.
// 예) 가격: 3000 → 2000 → 1000, 수량: 10 → sell(종목, 10주, 1000원)
TEST_F(TradingSystemFixture, sellNiceTiming_Success) {
	EXPECT_CALL(mockDriver, currentPrice("005930"))
		.WillOnce(Return(3000))
		.WillOnce(Return(2000))
		.WillOnce(Return(1000));
	EXPECT_CALL(mockDriver, sell("005930", 10, 1000)).Times(1);

	tradingSystem.sellNiceTiming("005930", 10);
}

// [자동매도] 실패 케이스 - 하락추세 아님
// 200ms 간격으로 3회 가격 조회 시 상승 추세(p1 < p2 < p3)이면 매도를 수행하지 않는다.
// 가격: 1000 → 2000 → 3000 (상승추세) → sell() 호출 없음
TEST_F(TradingSystemFixture, sellNiceTiming_Fail_NotFallingTrend) {
	EXPECT_CALL(mockDriver, currentPrice("005930"))
		.WillOnce(Return(1000))
		.WillOnce(Return(2000))
		.WillOnce(Return(3000));
	EXPECT_CALL(mockDriver, sell(_, _, _)).Times(0);

	tradingSystem.sellNiceTiming("005930", 10);
}

// [자동매도] 에러 케이스 - 빈 종목코드
// 종목코드에 빈 문자열("")을 전달하면 가격 조회 자체가 불가능하므로 즉시 false를 반환한다.
// currentPrice() 호출 없이 입력 검증 단계에서 차단되어야 불필요한 드라이버 접근을 막을 수 있다.
TEST_F(TradingSystemFixture, sellNiceTiming_Error_InvalidStockCode) {
	EXPECT_THAT(tradingSystem.sellNiceTiming("", 10), Eq(false));
}

// [자동매도] 에러 케이스 - 수량이 0
// 매도 수량이 0이면 매도할 주식이 없으므로 즉시 false를 반환한다.
// 0주 매도는 의미 없는 요청으로, 가격 조회 없이 입력 검증 단계에서 차단되어야 한다.
TEST_F(TradingSystemFixture, sellNiceTiming_Error_ZeroAmount) {
	EXPECT_THAT(tradingSystem.sellNiceTiming("005930", 0), Eq(false));
}

// [자동매도] 에러 케이스 - 수량이 음수
// 매도 수량이 음수이면 논리적으로 유효하지 않은 값이므로 즉시 false를 반환한다.
// 음수 수량은 잘못된 입력으로, 가격 조회 없이 입력 검증 단계에서 차단되어야 한다.
TEST_F(TradingSystemFixture, sellNiceTiming_Error_NegativeAmount) {
	EXPECT_THAT(tradingSystem.sellNiceTiming("005930", -10), Eq(false));
}
