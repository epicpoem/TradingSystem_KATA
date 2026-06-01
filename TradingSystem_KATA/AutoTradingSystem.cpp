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
		if (selectedStockerBrocker == nullptr) return false;
		if (id.empty() || pass.empty()) return false;
		selectedStockerBrocker->login(id, pass);
		return true;
	}

	// 매수
	bool buy(string code, int price, int amount) {
		return true;
	}

	//(종목코드, 가격, 수량) - 매도
	bool sell(string code, int price, int amount) {
		return true;
	}

	//현재가 확인
	int getPrice(string code) {
		if (selectedStockerBrocker == nullptr) return -1;
		if (code.empty()) return -1;
		int price = selectedStockerBrocker->currentPrice(code);
		if (price == 0) return -1;
		return price;
	}

	bool buyNiceTiming(string code, int netPrice) {
		return true;
	}

	bool sellNiceTiming(string code, int amount) {
		int prices[3];
		for (int i = 0; i < 3; i++) {
			prices[i] = selectedStockerBrocker->currentPrice(code);
			if (i < 2)
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		bool isDownTrend = (prices[0] > prices[1]) && (prices[1] > prices[2]);
		if (!isDownTrend)
			return false;

		selectedStockerBrocker->sell(code, amount, prices[2]);
		return true;
	}

private:
	StockerBrockerDriverInterface* selectedStockerBrocker = nullptr;
	bool ownsDriver = false;

};