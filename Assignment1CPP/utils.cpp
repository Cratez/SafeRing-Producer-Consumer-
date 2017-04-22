#include "utils.h"
int GetRand() {
	static std::default_random_engine sGenerator;
	static std::uniform_int_distribution<int> sDistribution(1, 1000);

	return sDistribution(sGenerator);
}