#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>

#include "StockerBrockerDriver.cpp"
#include "TimingStrategy.cpp"

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

	// 매매 전략 주입 (DI) - Runtime 중 교체 가능
	void setStrategy(ITimingStrategy* strat) {
		strategy = strat;
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
		if (selectedStockerBrocker == nullptr) return false;
		if (code.empty() || price <= 0 || amount <= 0) return false;
		selectedStockerBrocker->sell(code, amount, price);
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
		if (selectedStockerBrocker == nullptr) return false;
		if (strategy == nullptr) return false;
		if (code.empty()) return false;
		if (netPrice <= 0) return false;

		std::vector<int> prices;
		for (int i = 0; i < 3; i++) {
			prices.push_back(selectedStockerBrocker->currentPrice(code));
			if (i < 2)
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		if (!strategy->shouldBuy(prices)) return false;

		int amount = netPrice / prices.back();
		if (amount <= 0) return false;

		selectedStockerBrocker->buy(code, amount, prices.back());
		return true;
	}

	bool sellNiceTiming(string code, int amount) {
		if (selectedStockerBrocker == nullptr) return false;
		if (strategy == nullptr) return false;
		if (code.empty()) return false;
		if (amount <= 0) return false;

		std::vector<int> prices;
		for (int i = 0; i < 3; i++) {
			prices.push_back(selectedStockerBrocker->currentPrice(code));
			if (i < 2)
				std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}

		if (!strategy->shouldSell(prices)) return false;

		selectedStockerBrocker->sell(code, amount, prices.back());
		return true;
	}

private:
	StockerBrockerDriverInterface* selectedStockerBrocker = nullptr;
	bool ownsDriver = false;
	ITimingStrategy* strategy = nullptr;
};