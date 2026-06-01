#include <iostream>
#include <string>

#include "StockerBrockerDriver.cpp"

enum StockerBrocker{
	KIWER,
	NEMO,
};
using namespace std;

class AutoTradingSystem {
public:
	// 키워/네모 증권 선택
	bool selectStockBrocker(StockerBrocker sotocker) {
		return true;
	}
	
	//로그인
	bool Login(int id, string pass) {
		return true;
	}
		
	// 매수
	bool buy(int code, double price, int amount){
		return true;
	}

	//(종목코드, 가격, 수량) - 매도
	bool sell(int code, double price, int amount) {
		return true;
	}
	
	//현재가 확인
	bool getPrice(int code) {

	}

	bool buyNiceTiming(int code, double netPrice) {

	}

	bool sellNiceTiming(int code, int amount) {

	}

private : 
	StockerBrockerDriverInterface selectedStockerBrocker;

};