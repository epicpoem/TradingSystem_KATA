#pragma once
#include <vector>
#include <algorithm>

// 매매 전략 인터페이스 (Strategy 패턴)
// shouldBuy / shouldSell 에 가격 이력을 넘기면 매수/매도 여부를 반환한다.
struct ITimingStrategy {
	virtual bool shouldBuy(const std::vector<int>& priceHistory) = 0;
	virtual bool shouldSell(const std::vector<int>& priceHistory) = 0;
	virtual ~ITimingStrategy() = default;
};

// 3회 연속 상승/하락 추세 전략
class RisingTrendStrategy : public ITimingStrategy {
public:
	// 최근 3개 가격이 연속 상승(p1 < p2 < p3)이면 매수
	bool shouldBuy(const std::vector<int>& priceHistory) override {
		if ((int)priceHistory.size() < 3) return false;
		int n = (int)priceHistory.size();
		return priceHistory[n - 3] < priceHistory[n - 2] && priceHistory[n - 2] < priceHistory[n - 1];
	}

	// 최근 3개 가격이 연속 하락(p1 > p2 > p3)이면 매도
	bool shouldSell(const std::vector<int>& priceHistory) override {
		if ((int)priceHistory.size() < 3) return false;
		int n = (int)priceHistory.size();
		return priceHistory[n - 3] > priceHistory[n - 2] && priceHistory[n - 2] > priceHistory[n - 1];
	}
};

// 이동평균선 돌파 전략 (기본 기간: 3)
class MovingAverageStrategy : public ITimingStrategy {
public:
	explicit MovingAverageStrategy(int period = 3) : period(period) {}

	// 현재가가 직전 period개 이동평균 초과이면 매수
	bool shouldBuy(const std::vector<int>& priceHistory) override {
		if ((int)priceHistory.size() <= period) return false;
		return priceHistory.back() > calcMA(priceHistory);
	}

	// 현재가가 직전 period개 이동평균 미만이면 매도
	bool shouldSell(const std::vector<int>& priceHistory) override {
		if ((int)priceHistory.size() <= period) return false;
		return priceHistory.back() < calcMA(priceHistory);
	}

private:
	int period;

	double calcMA(const std::vector<int>& prices) const {
		int n = (int)prices.size();
		double sum = 0;
		for (int i = n - 1 - period; i < n - 1; i++) sum += prices[i];
		return sum / period;
	}
};

// 직전 N개 고점/저점 돌파 전략 (기본 윈도우: 3)
class BreakoutStrategy : public ITimingStrategy {
public:
	explicit BreakoutStrategy(int window = 3) : window(window) {}

	// 현재가가 직전 window개 고점을 돌파하면 매수
	bool shouldBuy(const std::vector<int>& priceHistory) override {
		if ((int)priceHistory.size() <= window) return false;
		int n = (int)priceHistory.size();
		int maxPrev = *std::max_element(
			priceHistory.begin() + (n - 1 - window),
			priceHistory.begin() + (n - 1));
		return priceHistory.back() > maxPrev;
	}

	// 현재가가 직전 window개 저점을 하향 돌파하면 매도
	bool shouldSell(const std::vector<int>& priceHistory) override {
		if ((int)priceHistory.size() <= window) return false;
		int n = (int)priceHistory.size();
		int minPrev = *std::min_element(
			priceHistory.begin() + (n - 1 - window),
			priceHistory.begin() + (n - 1));
		return priceHistory.back() < minPrev;
	}

private:
	int window;
};
