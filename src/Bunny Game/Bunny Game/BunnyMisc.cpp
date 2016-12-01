#include <ctime>
#include <random>

unsigned int RandFac(unsigned int andValue) {
	static std::mt19937 twister((unsigned int)time(NULL));
	return twister() & andValue;
}
int Rand2Fac(unsigned int andValue) {
	int retVal = (int)RandFac(andValue);
	return retVal - (andValue / 2);
}

float sintable(int a) {
	return sin((a & 1023) * 0.00613592314f); //convert from 1024 to tau
}
float costable(int a) {
	return sintable(a + 256);
}
