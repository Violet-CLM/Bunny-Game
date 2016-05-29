#pragma once
#include "BunnyObject.h"
#include "BunnyMisc.h"

class Bunny : public BunnyObject {
	bool KeyUp, KeyRight, KeyDown, KeyLeft, KeyFire, KeySelect, KeyRun, KeyJump;
	float SpeedX, AccelerationX;
	float SpeedY, AccelerationY;

	enum PlatformTypes {
		None, SwingingPlatform, SuckerTube
	};
	PlatformTypes platformType;
	float platform_relX, platform_relY, moveSpeedX, moveSpeedY, fixScrollX, quakeX, shiftPositionX;
	int fire, lastFire, lastDownAttack, freeze, invincibility, airBoard, helicopter, helicopterTotal, specialJump, dive, lastDive, hit, hDir, vDir, warpCounter, frogMorph, bossActive, vPole, swim, stop, stoned, stonedLen, spring, specialMove, slope, runDash, run, lastRun, rolling, quake, platform, ledgeWiggle, lastSpring, lastJump, idleTime, hPole, hang, vine, fly, fixStartX, downAttack, charCurr, characterIndex, beMoved, lastTilePosition, sugarRush, sucked, shieldType, shieldTime, morph, flicker, frameID, frameCount, animSpeed;
	bool goUp, goRight, goLeft, goDown, goFarDown, fixAnim;

	void GetInput(const KeyStates&);		//461C20
	void ProcessInput();					//435AC0
		void ProcessInputNoAirboard();
		void ProcessInputJumpFallStuff();
	void DoLandscapeCollision(GameState&);	//437D30
	void DoZoneDetection(Event);
	void AdjustRabbit();					//430ED0
		void ProcessActionFire();
	void ProcessAction();					//4348E0
	void AdjustViewpoint(GameState&) const;	//43E560

	void Behave(GameState&) override;
	//void Draw(Layer*) const override;
public:
	Bunny(ObjectStartPos& objStart);
};
