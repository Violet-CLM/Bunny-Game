#pragma once
#include "BunnyObject.h"

//todo move these somewhere better
//internal gametick speed is set at 70Hz
#define AISPEED			70

//no idea where the following are defined, but it's correct as far as I can tell
#define HITTIME			AISPEED
#define DOWNATTACKLEN	50
#define DOWNATTACKWAIT	40
#define FLASHTIME		5

class Bunny : public BunnyObject {
	bool KeyUp, KeyRight, KeyDown, KeyLeft, KeyFire, KeySelect, KeyRun, KeyJump;
	float SpeedX, AccelerationX;
	float SpeedY, AccelerationY;

	enum PlatformTypes {
		None, SwingingPlatform, SuckerTube
	};
	PlatformTypes platformType;
	float platform_relX, platform_relY, moveSpeedX, moveSpeedY, fixScrollX, quakeX, shiftPositionX;
	int fire, lastFire, lastDownAttack, freeze, invincibility, airBoard, helicopter, helicopterTotal, specialJump, dive, lastDive, hit, hDir, vDir, warpCounter, frogMorph, bossActive, vPole, swim, stop, stoned, stonedLen, spring, specialMove, slope, runDash, run, lastRun, rolling, quake, platform, ledgeWiggle, lastSpring, lastJump, idleTime, hPole, hang, vine, fly, fixStartX, downAttack, charCurr, characterIndex, beMoved;
	bool goUp, goRight, goLeft, goDown, goFarDown;

	void GetInput(const KeyStates&);		//461C20
	void ProcessInput();					//435AC0
		void ProcessInputNoAirboard();
		void ProcessInputJumpFallStuff();
	void DoLandscapeCollision(GameState&);	//437D30
	void DoZoneDetection();
	void ProcessAction();					//4348E0
	void AdjustViewpoint(GameState&) const;	//43E560

	void Behave(GameState&) override;
	//void Draw(Layer*) const override;
public:
	Bunny(ObjectStartPos& objStart);
};
