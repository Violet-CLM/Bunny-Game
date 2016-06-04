#pragma once
#include "BunnyObject.h"

class Pickup : public BunnyObject {
	using BunnyObject::BunnyObject;

protected:
	float BounceYOffset;

	void Draw(Layer*) const override;
	void Behave(GameState&) override;
public:
	Pickup(ObjectStartPos& objStart, int ai);
};
class AmmoPickup : public Pickup {
	int AmmoID;
	int AnimIDNormal;
	int AnimIDPoweredUp;

	void Behave(GameState&) override;
public:
	AmmoPickup(ObjectStartPos& objStart, int ai, int an, int anp);
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
	SpriteMode mode;
	void Draw(Layer*) const override;
public:
	Gem(ObjectStartPos& objStart, int c) : Pickup(objStart, 22), mode(SpriteMode(shaders[shader_NORMAL], c)) {} //todo gem shader instead
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
