#include <ctime>
#include <random>

unsigned int RandFac(unsigned int andValue) {
	static std::mt19937 twister((unsigned int)time(NULL));
	return twister() & andValue;
}
