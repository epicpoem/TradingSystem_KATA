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
		delete selectedStockerBrocker;
	}

	// 키워/네모 증권 선택
	bool selectStockBrocker(StockerBrocker stocker) {
		delete selectedStockerBrocker;
		if (stocker == KIWER)
			selectedStockerBrocker = new KiwerAdapter();
		else if (stocker == NEMO)
			selectedStockerBrocker = new NemoAdapter();
		else
			return false;
		return true;
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

	}

	bool buyNiceTiming(string code, int netPrice) {

	}

	bool sellNiceTiming(string code, int amount) {

	}

private:
	StockerBrockerDriverInterface* selectedStockerBrocker = nullptr;

};