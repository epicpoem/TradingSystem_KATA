#include "gmock/gmock.h"

#include "kiwer_api.cpp"
#include "nemo_api.cpp"

int main() {
	::testing::InitGoogleMock();
	return RUN_ALL_TESTS();
}
