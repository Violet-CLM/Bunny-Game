#pragma once
#include "Objects.h"
#include "Level.h"

class BunnyObject : public GameObject { //put deactivates, etc. code in here
	using GameObject::GameObject;
	void Draw(Layer*) const override;
protected:
	int DirectionX, DirectionY;
};

class Pickup : public BunnyObject {
	using BunnyObject::BunnyObject;

	float BounceYOffset;

	void Draw(Layer*) const override;
protected:
	void Behave(Level&) override;
public:
	Pickup(ObjectStartPos& objStart, int ai) : BunnyObject(objStart) {
		AnimID = ai;
		if (((int(OriginX) >> 5) & 1) != ((int(OriginY) >> 5) & 1)) //checkerboard
			DirectionX = -1;
	}
};
class AmmoPickup : public Pickup {
	int AmmoID;
	int AnimIDNormal;
	int AnimIDPoweredUp;

	void Behave(Level&) override;
public:
	AmmoPickup(ObjectStartPos& objStart, int ai, int an, int anp) : Pickup(objStart, ai), AmmoID(ai), AnimIDNormal(an), AnimIDPoweredUp(anp) {}
};
class Food : public Pickup {
	using Pickup::Pickup;
};

class ExtraLife : public Pickup {
public:
	ExtraLife(ObjectStartPos& objStart) : Pickup(objStart, 0) {}
};
class Carrot : public Pickup {
public:
	Carrot(ObjectStartPos& objStart) : Pickup(objStart, 21) {}
};
class Gem : public Pickup {
	int color;
public:
	Gem(ObjectStartPos& objStart, int c) : Pickup(objStart, 22), color(c) {}
};
class FastFire : public Pickup {
public:
	FastFire(ObjectStartPos& objStart) : Pickup(objStart, 29) {} //todo
};
class SilverCoin : public Pickup {
public:
	SilverCoin(ObjectStartPos& objStart) : Pickup(objStart, 84) {}
};
class GoldCoin : public Pickup {
public:
	GoldCoin(ObjectStartPos& objStart) : Pickup(objStart, 37) {}
};
class FlyCarrot : public Pickup {
public:
	FlyCarrot(ObjectStartPos& objStart) : Pickup(objStart, 40) {}
};
class FreezeEnemies : public Pickup {
public:
	FreezeEnemies(ObjectStartPos& objStart) : Pickup(objStart, 42) {}
};
class Invincibility : public Pickup {
public:
	Invincibility(ObjectStartPos& objStart) : Pickup(objStart, 72) {}
};
class FullEnergy : public Pickup {
public:
	FullEnergy(ObjectStartPos& objStart) : Pickup(objStart, 82) {}
};
class StopWatch : public Pickup {
public:
	StopWatch(ObjectStartPos& objStart) : Pickup(objStart, 87) {}
};


class Bee : public BunnyObject {
	using BunnyObject::BunnyObject;
	int Counter;
	void Behave(Level&) override;
};