#pragma once

//internal gametick speed is set at 70Hz
#define AISPEED			70
#define HITTIME			AISPEED
#define DOWNATTACKLEN	50
#define DOWNATTACKWAIT	40
#define FLASHTIME		5

#define GEM_COLOR_COUNT 4
#define START_HEALTH 5
#define START_LIVES 3

#define ApproachZero(a) if (a > 0) --a; else if (a < 0) ++a
#define LowerToZero(a) if (a) --a
#define ApproachZeroByUnit(a, u) if (a > u) a -= u; else if (a < -u) a += u; else a = 0
#define ApproachConstantByConstant(value, target, unit) if (value < target) value += unit; else if (value > target) value -= unit//; else value = target
#define Minimize(value, limit) if (value > limit) value = limit
#define Maximize(value, limit) if (value < limit) value = limit
#define LimitToRange(value, lower, upper) Maximize(value, lower); else Minimize(value, upper)
#define LimitTo(value, limit) LimitToRange(value, -limit, limit)

unsigned int RandFac(unsigned int andValue);
int Rand2Fac(unsigned int andValue);

float sintable(int);
float costable(int);
#define sinTable sintable
#define cosTable costable

enum class Difficulty {
	Easy, Medium, Hard, Turbo
};
extern Difficulty GameDifficulty;
