#include <iostream>
#include <string>
#include "kiwer_api.cpp"
#include "nemo_api.cpp"

struct StockerBrockerDriverInterface {
	virtual void login(std::string ID, std::string password) = 0;
	virtual void buy(std::string stockCode, int count, int price) = 0;
	virtual void sell(std::string stockCode, int count, int price) = 0;
	virtual int currentPrice(std::string stockCode) = 0;
	virtual ~StockerBrockerDriverInterface() = default;
};

class KiwerAdapter : public StockerBrockerDriverInterface {
public:
	void login(std::string ID, std::string password) override {
		api.login(ID, password);
	}
	void buy(std::string stockCode, int count, int price) override {
		api.buy(stockCode, count, price);
	}
	void sell(std::string stockCode, int count, int price) override {
		api.sell(stockCode, count, price);
	}
	int currentPrice(std::string stockCode) override {
		return api.currentPrice(stockCode);
	}
private:
	KiwerAPI api;
};

class NemoAdapter : public StockerBrockerDriverInterface {
public:
	void login(std::string ID, std::string password) override {
		api.certification(ID, password);
	}
	void buy(std::string stockCode, int count, int price) override {
		api.purchasingStock(stockCode, price, count);
	}
	void sell(std::string stockCode, int count, int price) override {
		api.sellingStock(stockCode, price, count);
	}
	int currentPrice(std::string stockCode) override {
		return api.getMarketPrice(stockCode, 1);
	}
private:
	NemoAPI api;
};