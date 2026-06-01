#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include "StockerBrockerDriver.cpp"

// 로그 인터페이스
struct ILogger {
	virtual void log(const std::string& message) = 0;
	virtual ~ILogger() = default;
};

// 주문 커맨드 인터페이스 (Command 패턴)
// 주문을 객체로 캡슐화하여 예약 실행 / 로깅에 활용한다.
struct IOrderCommand {
	virtual void execute() = 0;
	virtual std::string getDescription() const = 0;
	virtual ~IOrderCommand() = default;
};

// 매수 주문 커맨드
// execute() 호출 시 드라이버의 buy(code, amount, price) 를 실행한다.
class BuyOrder : public IOrderCommand {
public:
	BuyOrder(StockerBrockerDriverInterface* driver, std::string code, int price, int amount)
		: driver(driver), code(code), price(price), amount(amount) {}

	void execute() override {
		driver->buy(code, amount, price);
	}

	std::string getDescription() const override {
		return "BUY " + code + " price:" + std::to_string(price) + " amount:" + std::to_string(amount);
	}

private:
	StockerBrockerDriverInterface* driver;
	std::string code;
	int price;
	int amount;
};

// 매도 주문 커맨드
// execute() 호출 시 드라이버의 sell(code, amount, price) 를 실행한다.
class SellOrder : public IOrderCommand {
public:
	SellOrder(StockerBrockerDriverInterface* driver, std::string code, int price, int amount)
		: driver(driver), code(code), price(price), amount(amount) {}

	void execute() override {
		driver->sell(code, amount, price);
	}

	std::string getDescription() const override {
		return "SELL " + code + " price:" + std::to_string(price) + " amount:" + std::to_string(amount);
	}

private:
	StockerBrockerDriverInterface* driver;
	std::string code;
	int price;
	int amount;
};

// 예약 주문 스케줄러
// scheduleOrder() 로 주문을 큐에 등록하고,
// runPending() 호출 시 현재 시각 이상의 주문을 자동 실행하며 실행 전후 로그를 남긴다.
class OrderScheduler {
public:
	explicit OrderScheduler(ILogger* logger) : logger(logger) {}

	bool scheduleOrder(IOrderCommand* order, time_t executeTime) {
		if (order == nullptr) return false;
		scheduledOrders.push_back({ order, executeTime });
		return true;
	}

	void runPending(time_t currentTime) {
		std::vector<ScheduledEntry> remaining;
		for (auto& entry : scheduledOrders) {
			if (currentTime >= entry.executeTime) {
				logger->log("[START] " + entry.order->getDescription());
				entry.order->execute();
				logger->log("[END] " + entry.order->getDescription());
			} else {
				remaining.push_back(entry);
			}
		}
		scheduledOrders = remaining;
	}

private:
	struct ScheduledEntry {
		IOrderCommand* order;
		time_t executeTime;
	};
	std::vector<ScheduledEntry> scheduledOrders;
	ILogger* logger;
};
