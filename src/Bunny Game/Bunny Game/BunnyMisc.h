#pragma once

//internal gametick speed is set at 70Hz
#define AISPEED			70
#define HITTIME			AISPEED
#define DOWNATTACKLEN	50
#define DOWNATTACKWAIT	40
#define FLASHTIME		5

#define ApproachZero(a) if (a > 0) --a; else if (a < 0) ++a
#define LowerToZero(a) if (a) --a

unsigned int RandFac(unsigned int andValue);

float sintable(int);
float costable(int);
#define sinTable sintable
#define cosTable costable
