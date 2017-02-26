#pragma once
#include "BunnyObject.h"
#include "BunnyMisc.h"
#include "BunnyWeapons.h"

#define MAXLOCALPLAYERS 1
#define MAXPLAYERTRACE 20

enum char2Indices { char2JAZZ, char2SPAZ, char2LORI }; //todo better solution elsewhere

//this struct holds all the player variables that hold constant across level boundaries/when restarting from a checkpoint
class Bunny;
struct Player {
	Bunny* Object;

	int Score, Lives, Coins, FireSpeed, CharacterIndex;
	int Food; //Food needs special treatment because it doesn't reset upon death
	std::array<bool, Weapon::LAST> Powerups;
	std::array<int, Weapon::LAST> Ammo;
	std::array<int, GEM_COLOR_COUNT> Gems;
	
	Player() {
		memset(this, 0, sizeof(*this));
		Lives = START_LIVES;
		FireSpeed = AISPEED / 2;
	}
	Player& operator=(const Player& other) {
		memcpy(this, &other, sizeof(*this));
		return *this;
	}
};
extern std::array<Player, MAXLOCALPLAYERS> Players;

class Bunny : public BunnyObject {
	friend class BunnyObject;

	bool KeyUp, KeyRight, KeyDown, KeyLeft, KeyFire, KeySelect, KeyRun, KeyJump;
	bool WasPressingKeySelectLastGameTick;
	float SpeedX, AccelerationX, OldSpeedX, WarpTargetPositionX;
	float SpeedY, AccelerationY, OldSpeedY, WarpTargetPositionY;

	enum PlatformTypes {
		None, SwingingPlatform, SuckerTube
	};
	PlatformTypes platformType;
	float platform_relX, platform_relY, moveSpeedX, moveSpeedY, fixScrollX, quakeX, shiftPositionX, poleSpeed, fireAngle;
	int playerID, fire, lastFire, lastDownAttack, airBoard, helicopter, helicopterTotal, specialJump, dive, lastDive, hit, DirectionKeyX, DirectionKeyY, warpCounter, frogMorph, bossActive, vPole, swim, stop, stoned, stonedLen, spring, specialMove, slope, runDash, run, lastRun, rolling, quake, platform, ledgeWiggle, lastSpring, lastJump, idleTime, hPole, hang, vine, fixStartX, downAttack, charCurr, beMoved, lastTilePosition, sugarRush, sucked, shieldType, shieldTime, morph, flicker, frameCount, animSpeed, warpFall, warpArea, viewSkipAverage, skid, pushObject, push, lookVP, lookVPAmount, lift, lastPush, lastLookVP, idleTrail, idleExtra, idleAnim, health, fireDirection;
	bool goUp, goRight, goLeft, goDown, goFarDown, fixAnim, fireHold;
	Event* LastSuckerTube;
	sf::Vector2f Trace[MAXPLAYERTRACE]; int TraceStartIndex = 0; unsigned int TraceLength = 0;
	unsigned int LightingTarget;

	void GetInput(const KeyStates&);		//461C20
	void ProcessInput();					//435AC0
		void DepleteAmmo();
		void ProcessFireInput();
		void ProcessSelectInput();
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
		void AddBullet();
		void AddBullet(sf::Vector2f, float, bool=true);
		void AddSingleBullet(float, sf::Vector2f, EventID, bool=true, float=0);
		void AddSingleBullet(float, sf::Vector2f, EventID, float,float, bool=false);
		bool ProcessActionFire();
		bool ProcessActionSpecialMove();
	void AdjustViewpoint(GameState&) const;	//43E560

	void DisableSpecialMove();

	void Behave(GameState&) override;
	void Draw(Layer*) const override;
public:
	enum AttackTypes {
		NotAttacking, Buttstomp, SugarRush, SpecialAttack, RunThroughFrozenEnemy
	};
	Bunny(ObjectStartPos& objStart, int);

	void EatFood();
	bool Hurt(unsigned int = 1);
	bool IsHurt() const { return flicker != 0 || hit > 0; }
	AttackTypes GetAttackType(bool = false) const;
	void HitEnemyUsingAttackType(AttackTypes);
	void AddToInvincibilityDuration(int);

	int Health;
	int freeze, invincibility, fly, fireType;
	const char* HelpString; int HelpStringCounter = 0;
	Player PlayerProperties;
};
