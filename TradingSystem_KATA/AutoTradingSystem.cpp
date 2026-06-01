#include <iostream>
#include <string>

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
		return true;
	}

	bool sellNiceTiming(string code, int amount) {
		return true;
	}

private:
	StockerBrockerDriverInterface* selectedStockerBrocker = nullptr;
	bool ownsDriver = false;

};