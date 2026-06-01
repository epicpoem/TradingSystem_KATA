#include <gmock/gmock.h>
#include "OrderCommand.cpp"

using namespace testing;

namespace {
	class MockStockerBrockerDriver : public StockerBrockerDriverInterface {
	public:
		MOCK_METHOD(void, login, (std::string ID, std::string password), (override));
		MOCK_METHOD(void, buy, (std::string stockCode, int count, int price), (override));
		MOCK_METHOD(void, sell, (std::string stockCode, int count, int price), (override));
		MOCK_METHOD(int, currentPrice, (std::string stockCode), (override));
	};

	class MockOrderCommand : public IOrderCommand {
	public:
		MOCK_METHOD(void, execute, (), (override));
		MOCK_METHOD(std::string, getDescription, (), (const, override));
	};

	class MockLogger : public ILogger {
	public:
		MOCK_METHOD(void, log, (const std::string& message), (override));
	};
}

class OrderSchedulerFixture : public Test {
public:
	NiceMock<MockLogger> mockLogger;
	NiceMock<MockStockerBrockerDriver> mockDriver;
	OrderScheduler scheduler{ &mockLogger };
};

// =============================================
// scheduleOrder 테스트
// =============================================

// [예약 등록] 성공 케이스
// 유효한 주문을 등록하면 true를 반환한다.
TEST_F(OrderSchedulerFixture, scheduleOrder_Success) {
	NiceMock<MockOrderCommand> mockCmd;
	EXPECT_THAT(scheduler.scheduleOrder(&mockCmd, 1000), Eq(true));
}

// [예약 등록] 실패 케이스 - null 주문
// null 주문을 등록하면 false를 반환하며, 내부 큐에 저장되지 않아야 한다.
TEST_F(OrderSchedulerFixture, scheduleOrder_Fail_NullCommand) {
	EXPECT_THAT(scheduler.scheduleOrder(nullptr, 1000), Eq(false));
}

// =============================================
// runPending 실행 타이밍 테스트
// =============================================

// [예약 실행] 성공 케이스 - 예약 시각 도달 시 execute() 호출됨
// currentTime == executeTime 이면 주문이 실행되어야 한다.
TEST_F(OrderSchedulerFixture, runPending_ExecutesOrderAtExactScheduledTime) {
	NiceMock<MockOrderCommand> mockCmd;
	ON_CALL(mockCmd, getDescription()).WillByDefault(Return("TEST ORDER"));
	scheduler.scheduleOrder(&mockCmd, 1000);

	EXPECT_CALL(mockCmd, execute()).Times(1);
	scheduler.runPending(1000);
}

// [예약 실행] 성공 케이스 - 예약 시각 이후에도 execute() 호출됨
// currentTime > executeTime 이면 늦게 실행해도 주문이 처리되어야 한다.
TEST_F(OrderSchedulerFixture, runPending_ExecutesOrderAfterScheduledTime) {
	NiceMock<MockOrderCommand> mockCmd;
	ON_CALL(mockCmd, getDescription()).WillByDefault(Return("TEST ORDER"));
	scheduler.scheduleOrder(&mockCmd, 1000);

	EXPECT_CALL(mockCmd, execute()).Times(1);
	scheduler.runPending(2000);
}

// [예약 실행] 실패 케이스 - 예약 시각 이전에는 execute() 미호출
// currentTime < executeTime 이면 아직 시각이 되지 않았으므로 실행하지 않는다.
TEST_F(OrderSchedulerFixture, runPending_DoesNotExecuteBeforeScheduledTime) {
	NiceMock<MockOrderCommand> mockCmd;
	ON_CALL(mockCmd, getDescription()).WillByDefault(Return("TEST ORDER"));
	scheduler.scheduleOrder(&mockCmd, 1000);

	EXPECT_CALL(mockCmd, execute()).Times(0);
	scheduler.runPending(500);
}

// =============================================
// runPending 로깅 테스트
// =============================================

// [로깅] 성공 케이스 - 실행 전후 [START]/[END] 로그가 순서대로 기록됨
// execute() 호출 전에 [START], 호출 후에 [END] 로그가 남아야 한다.
TEST_F(OrderSchedulerFixture, runPending_LogsStartBeforeAndEndAfterExecution) {
	MockLogger strictLogger;
	OrderScheduler logScheduler{ &strictLogger };
	NiceMock<MockOrderCommand> mockCmd;
	ON_CALL(mockCmd, getDescription()).WillByDefault(Return("TEST ORDER"));
	logScheduler.scheduleOrder(&mockCmd, 1000);

	InSequence seq;
	EXPECT_CALL(strictLogger, log(HasSubstr("[START]"))).Times(1);
	EXPECT_CALL(mockCmd, execute()).Times(1);
	EXPECT_CALL(strictLogger, log(HasSubstr("[END]"))).Times(1);

	logScheduler.runPending(1000);
}

// =============================================
// runPending 큐 관리 테스트
// =============================================

// [큐 관리] 에러 케이스 - 이미 실행된 주문은 재실행되지 않음
// runPending() 을 두 번 호출해도 이미 처리된 주문은 다시 실행되지 않아야 한다.
TEST_F(OrderSchedulerFixture, runPending_DoesNotReExecuteCompletedOrder) {
	NiceMock<MockOrderCommand> mockCmd;
	ON_CALL(mockCmd, getDescription()).WillByDefault(Return("TEST ORDER"));
	scheduler.scheduleOrder(&mockCmd, 1000);

	EXPECT_CALL(mockCmd, execute()).Times(1);
	scheduler.runPending(1000);
	scheduler.runPending(2000);
}

// [큐 관리] 성공 케이스 - 시각이 된 여러 주문이 모두 실행됨
// 동일 시각에 등록된 주문이 여럿이면 모두 실행되어야 한다.
TEST_F(OrderSchedulerFixture, runPending_ExecutesAllReadyOrders) {
	NiceMock<MockOrderCommand> mockCmd1, mockCmd2;
	ON_CALL(mockCmd1, getDescription()).WillByDefault(Return("ORDER1"));
	ON_CALL(mockCmd2, getDescription()).WillByDefault(Return("ORDER2"));
	scheduler.scheduleOrder(&mockCmd1, 1000);
	scheduler.scheduleOrder(&mockCmd2, 1000);

	EXPECT_CALL(mockCmd1, execute()).Times(1);
	EXPECT_CALL(mockCmd2, execute()).Times(1);
	scheduler.runPending(1000);
}

// [큐 관리] 에러 케이스 - 시각이 안 된 주문은 건너뛰고 시각이 된 주문만 실행됨
// 여러 주문 중 시각에 도달한 것만 선택적으로 실행해야 한다.
TEST_F(OrderSchedulerFixture, runPending_ExecutesOnlyReadyOrders) {
	NiceMock<MockOrderCommand> earlyCmd, laterCmd;
	ON_CALL(earlyCmd, getDescription()).WillByDefault(Return("EARLY"));
	ON_CALL(laterCmd, getDescription()).WillByDefault(Return("LATER"));
	scheduler.scheduleOrder(&earlyCmd, 1000);
	scheduler.scheduleOrder(&laterCmd, 5000);

	EXPECT_CALL(earlyCmd, execute()).Times(1);
	EXPECT_CALL(laterCmd, execute()).Times(0);
	scheduler.runPending(1000);
}

// =============================================
// BuyOrder / SellOrder 단위 테스트
// =============================================

// [BuyOrder] 성공 케이스 - execute() 호출 시 driver->buy(code, count, price) 가 호출됨
// BuyOrder(driver, code, price, amount) → driver->buy(code, amount, price) 로 순서 변환됨에 주의
TEST(BuyOrderTest, execute_CallsDriverBuy_WithCorrectParams) {
	NiceMock<MockStockerBrockerDriver> mockDriver;
	BuyOrder order(&mockDriver, "005930", 50000, 10);

	EXPECT_CALL(mockDriver, buy("005930", 10, 50000)).Times(1);
	order.execute();
}

// [BuyOrder] getDescription 성공 케이스 - 종목코드/가격/수량이 포함된 설명을 반환함
TEST(BuyOrderTest, getDescription_ContainsStockCodeAndParams) {
	NiceMock<MockStockerBrockerDriver> mockDriver;
	BuyOrder order(&mockDriver, "005930", 50000, 10);

	EXPECT_THAT(order.getDescription(), HasSubstr("005930"));
	EXPECT_THAT(order.getDescription(), HasSubstr("50000"));
	EXPECT_THAT(order.getDescription(), HasSubstr("10"));
}

// [SellOrder] 성공 케이스 - execute() 호출 시 driver->sell(code, count, price) 가 호출됨
TEST(SellOrderTest, execute_CallsDriverSell_WithCorrectParams) {
	NiceMock<MockStockerBrockerDriver> mockDriver;
	SellOrder order(&mockDriver, "005930", 55000, 10);

	EXPECT_CALL(mockDriver, sell("005930", 10, 55000)).Times(1);
	order.execute();
}

// [SellOrder] getDescription 성공 케이스 - 종목코드/가격/수량이 포함된 설명을 반환함
TEST(SellOrderTest, getDescription_ContainsStockCodeAndParams) {
	NiceMock<MockStockerBrockerDriver> mockDriver;
	SellOrder order(&mockDriver, "005930", 55000, 10);

	EXPECT_THAT(order.getDescription(), HasSubstr("005930"));
	EXPECT_THAT(order.getDescription(), HasSubstr("55000"));
	EXPECT_THAT(order.getDescription(), HasSubstr("10"));
}
