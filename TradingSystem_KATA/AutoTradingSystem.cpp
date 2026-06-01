#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "StockerBrockerDriver.cpp"

enum StockerBrocker {
	KIWER,
	NEMO,
};
using namespace std;

class AutoTradingSystem {
public:
	~AutoTradingSystem() {
		if (ownsDriver) delete selectedStockerBrocker;
	}

	// 키워/네모 증권 선택
	bool selectStockBrocker(StockerBrocker stocker) {
		if (ownsDriver) delete selectedStockerBrocker;
		if (stocker == KIWER)
			selectedStockerBrocker = new KiwerAdapter();
		else if (stocker == NEMO)
			selectedStockerBrocker = new NemoAdapter();
		else
			return false;
		ownsDriver = true;
		return true;
	}

	// 테스트용 mock 주입 오버로드 (소유권 없음 - delete 하지 않음)
	void selectStockBrocker(StockerBrockerDriverInterface* driver) {
		if (ownsDriver) delete selectedStockerBrocker;
		selectedStockerBrocker = driver;
		ownsDriver = false;
	}

	//로그인
	bool login(string id, string pass) {
		return true;
	}

	// 매수
	bool buy(string code, int price, int amount) {
		if (selectedStockerBrocker == nullptr) return false;
		if (code.empty()) return false;
		if (price <= 0) return false;
		if (amount <= 0) return false;

		// API(가격, 수량) → 드라이버(수량, 가격) 순서로 역전하여 전달
		selectedStockerBrocker->buy(code, amount, price);
		return true;
	}

	//(종목코드, 가격, 수량) - 매도
	bool sell(string code, int price, int amount) {
		return true;
	}

	//현재가 확인
	bool getPrice(string code) {
		return true;
	}

	bool buyNiceTiming(string code, int netPrice) {
		if (selectedStockerBrocker == nullptr) return false;
		if (code.empty()) return false;
		if (netPrice <= 0) return false;

		// 200ms 간격으로 3회 시세를 조회한다.
		int p1 = selectedStockerBrocker->currentPrice(code);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		int p2 = selectedStockerBrocker->currentPrice(code);
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		int p3 = selectedStockerBrocker->currentPrice(code);

		// 상승 추세(p1 < p2 < p3)가 아니면 매수하지 않는다.
		if (!(p1 < p2 && p2 < p3)) return false;

		// 총금액을 마지막 가격으로 나눈 최대 수량만큼 마지막 가격에 매수한다.
		int amount = netPrice / p3;
		if (amount <= 0) return false;

		selectedStockerBrocker->buy(code, amount, p3);
		return true;
	}

	bool sellNiceTiming(string code, int amount) {
		return true;
	}

private:
	StockerBrockerDriverInterface* selectedStockerBrocker = nullptr;
	bool ownsDriver = false;

};