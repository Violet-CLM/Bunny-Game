#pragma once
#include "Objects.h"
#include "Level.h"

class BunnyObject : public GameObjectBehavior { //put deactivates, etc. code in here
	using GameObjectBehavior::GameObjectBehavior;
	void Draw(Layer*) const override;
protected:
	int DirectionX, DirectionY;
};
#include "Windows.h"
#include "Misc.h"
class Pickup : public BunnyObject {
	using BunnyObject::BunnyObject;

	float BounceYOffset;

	void Draw(Layer*) const override;
protected:
	void Behave(Level&) override;
public:
	Pickup(GameObject* objBase, int ai) : BunnyObject(objBase) {
		DetermineFrame(ai, 0);
		if (((int(BasicProperties->OriginX) >> 5) & 1) != ((int(BasicProperties->OriginY) >> 5) & 1)) //checkerboard
			DirectionX = -1;
	}
};
class AmmoPickup : public Pickup {
	int AmmoID;
	int AnimID;
	int AnimIDPoweredUp;

	void Behave(Level&) override;
public:
	AmmoPickup(GameObject* objBase, int ai, int an, int anp) : Pickup(objBase, ai), AmmoID(ai), AnimID(an), AnimIDPoweredUp(anp) {}
};
class Food : public Pickup {
	using Pickup::Pickup;
};

class ExtraLife : public Pickup {
public:
	ExtraLife(GameObject* objBase) : Pickup(objBase, 0) {}
};
class Carrot : public Pickup {
public:
	Carrot(GameObject* objBase) : Pickup(objBase, 21) {}
};
class Gem : public Pickup {
	int color;
public:
	Gem(GameObject* objBase, int c) : Pickup(objBase, 22), color(c) {}
};
class FastFire : public Pickup {
public:
	FastFire(GameObject* objBase) : Pickup(objBase, 29) {} //todo
};
class SilverCoin : public Pickup {
public:
	SilverCoin(GameObject* objBase) : Pickup(objBase, 84) {}
};
class GoldCoin : public Pickup {
public:
	GoldCoin(GameObject* objBase) : Pickup(objBase, 37) {}
};
class FlyCarrot : public Pickup {
public:
	FlyCarrot(GameObject* objBase) : Pickup(objBase, 40) {}
};
class FreezeEnemies : public Pickup {
public:
	FreezeEnemies(GameObject* objBase) : Pickup(objBase, 42) {}
};
class Invincibility : public Pickup {
public:
	Invincibility(GameObject* objBase) : Pickup(objBase, 72) {}
};
class FullEnergy : public Pickup {
public:
	FullEnergy(GameObject* objBase) : Pickup(objBase, 82) {}
};
class StopWatch : public Pickup {
public:
	StopWatch(GameObject* objBase) : Pickup(objBase, 87) {}
};


class Bee : public BunnyObject {
	using BunnyObject::BunnyObject;
	int Counter;
	void Behave(Level&) override;
};