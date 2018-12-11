#pragma once
#include "Bunny.h"
#include "BunnyObject.h"
#include "BunnyShaders.h"
#include "BunnySamples.h"

class Pickup : public BunnyObject {
	using BunnyObject::BunnyObject;
	friend class BunnyObject;

	unsigned int TimeTillCollectable = 0;
	bool Pickup::InMotion() const;
protected:
	float BounceYOffset, SpeedX, SpeedY;
	unsigned int Sample;
	
	void Draw(Layer*) const override;
	void Behave(GameState&) override;
	virtual void Collected(Bunny&) const {} //called by HitBy
	void HitBy(GameObject&) override;
public:
	Pickup(ObjectStartPos& objStart, int ai, int s = Samples::sCommon_PICKUP1);
	static int ExplosionSetID;
};
class AmmoPickup : public Pickup {
	int AmmoID;
	int AnimIDNormal;
	int AnimIDPoweredUp;
	unsigned int Shift; //for toaster

	void Behave(GameState&) override;
	void Collected(Bunny&) const override;
	void HitBy(GameObject&) override; //doesn't get collected if player is already at full ammo
public:
	AmmoPickup(ObjectStartPos& objStart, int ai);
};
class Food : public Pickup {
	void Collected(Bunny&) const override;
public:
	Food(ObjectStartPos& objStart, int ai, bool isDrink = false) : Pickup(objStart, ai, (!isDrink ? Samples::sCommon_EAT1 : Samples::sCommon_DRINKSPAZZ1) + RandFac(3)) {}
};

class ExtraLife : public Pickup {
	void Collected(Bunny&) const override;
public:
	ExtraLife(ObjectStartPos& objStart) : Pickup(objStart, 0, Samples::sCommon_HARP1) {
		DirectionX = 1;
	}
};
class Carrot : public Pickup {
	void Collected(Bunny&) const override;
	void HitBy(GameObject&) override; //doesn't get collected if player is already at full health
public:
	Carrot(ObjectStartPos& objStart) : Pickup(objStart, 21, Samples::sCommon_EAT1 + RandFac(3)) {}
};
class Gem : public Pickup {
	int GemColor;
	void Collected(Bunny&) const override;
	SpriteMode mode;
	void Draw(Layer*) const override;
public:
	Gem(ObjectStartPos& objStart, int c) : Pickup(objStart, 22), GemColor(c % GEM_COLOR_COUNT), mode(SpriteModeGem(GemColor)) {}
};
class FastFire : public Pickup {
public:
	FastFire(ObjectStartPos& objStart) : Pickup(objStart, 29, Samples::sCommon_PICKUPW1) {} //todo
};
class SilverCoin : public Pickup {
	void Collected(Bunny&) const override;
public:
	SilverCoin(ObjectStartPos& objStart) : Pickup(objStart, 84, Samples::sCommon_COIN) {}
};
class GoldCoin : public Pickup {
	void Collected(Bunny&) const override;
public:
	GoldCoin(ObjectStartPos& objStart) : Pickup(objStart, 37, Samples::sCommon_COIN) {}
};
class FlyCarrot : public Pickup {
	void Collected(Bunny&) const override;
public:
	FlyCarrot(ObjectStartPos& objStart) : Pickup(objStart, 40) {}
};
class FreezeEnemies : public Pickup {
	void Collected(Bunny&) const override;
public:
	FreezeEnemies(ObjectStartPos& objStart) : Pickup(objStart, 42) {}
};
class Invincibility : public Pickup {
	void Collected(Bunny&) const override;
public:
	Invincibility(ObjectStartPos& objStart) : Pickup(objStart, 72) {}
};
class FullEnergy : public Pickup {
	void Collected(Bunny&) const override;
	void HitBy(GameObject&) override; //doesn't get collected if player is already at full health
public:
	FullEnergy(ObjectStartPos& objStart) : Pickup(objStart, 82, 0) {}
};
class StopWatch : public Pickup {
public:
	StopWatch(ObjectStartPos& objStart) : Pickup(objStart, 87) {}
};
