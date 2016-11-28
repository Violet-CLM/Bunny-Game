#pragma once
#include "BunnyObject.h"
#include "BunnyMisc.h"

#define MAXLOCALPLAYERS 1

//this struct holds all the player variables that hold constant across level boundaries/when restarting from a checkpoint
class Bunny;
struct Player {
	Bunny* Object;

	int Score, Lives, Coins, CharacterIndex;
	int Food; //Food needs special treatment because it doesn't reset upon death
	std::array<bool, WEAPON_COUNT> Powerups;
	std::array<int, WEAPON_COUNT> Ammo;
	std::array<int, GEM_COLOR_COUNT> Gems;
	
	Player() {
		memset(this, 0, sizeof(*this));
		Lives = START_LIVES;
	}
	Player& operator=(const Player& other) {
		memcpy(this, &other, sizeof(*this));
		return *this;
	}
};
extern std::array<Player, MAXLOCALPLAYERS> Players;

class Bunny : public BunnyObject {
	bool KeyUp, KeyRight, KeyDown, KeyLeft, KeyFire, KeySelect, KeyRun, KeyJump;
	float SpeedX, AccelerationX, OldSpeedX, WarpTargetPositionX;
	float SpeedY, AccelerationY, OldSpeedY, WarpTargetPositionY;

	enum PlatformTypes {
		None, SwingingPlatform, SuckerTube
	};
	PlatformTypes platformType;
	float platform_relX, platform_relY, moveSpeedX, moveSpeedY, fixScrollX, quakeX, shiftPositionX, poleSpeed;
	int playerID, fire, lastFire, lastDownAttack, airBoard, helicopter, helicopterTotal, specialJump, dive, lastDive, hit, DirectionKeyX, DirectionKeyY, warpCounter, frogMorph, bossActive, vPole, swim, stop, stoned, stonedLen, spring, specialMove, slope, runDash, run, lastRun, rolling, quake, platform, ledgeWiggle, lastSpring, lastJump, idleTime, hPole, hang, vine, fixStartX, downAttack, charCurr, beMoved, lastTilePosition, sugarRush, sucked, shieldType, shieldTime, morph, flicker, frameCount, animSpeed, warpFall, warpArea, viewSkipAverage, skid, pushObject, push, lookVP, lookVPAmount, lift, lastPush, lastLookVP, idleTrail, idleExtra, idleAnim, health, fireSpeed, fireDirection;
	bool goUp, goRight, goLeft, goDown, goFarDown, fixAnim;
	Event* LastSuckerTube;

	void GetInput(const KeyStates&);		//461C20
	void ProcessInput();					//435AC0
		void ProcessInputNoAirboard();
		void ProcessInputJumpFallStuff();
		void ProcessInputStuffWithFlyAndSwim();
	void DoLandscapeCollision(GameState&);	//437D30
	void DoZoneDetection(Event&, unsigned int);
		inline void CenterInTile(bool=true,bool=true);
	void AdjustRabbit(unsigned int);		//430ED0
		inline bool UsesAnimation(int) const;
		inline void Bunny::AssignAnimation(int);
		inline void Bunny::AssignAnimation(int, int);
		inline void Bunny::AssignAnimation(int, int, bool, int = 0);
	void PoleSamples() const;
	void ProcessAction(unsigned int);		//4348E0
		void ProcessActionFire();
		bool ProcessActionSpecialMove();
	void AdjustViewpoint(GameState&) const;	//43E560

	void Behave(GameState&) override;
	//void Draw(Layer*) const override;
public:
	Bunny(ObjectStartPos& objStart);

	void EatFood();

	int Health;
	int freeze, invincibility, fly;
	Player PlayerProperties;
};
