#pragma once
#include "BunnyObject.h"

class Bee : public Interactive {
	int Counter;
	const Bunny* Target;
	float
		ApproachSpeedX, ApproachSpeedY, //can't use the same pair of floats for each state, because bees start out attacking you at the same angle they were at the last time they finished attacking you
		RetreatSpeedX, RetreatSpeedY;
	void Move(GameState&) override;
public:
	Bee(ObjectStartPos&);
};

class NormalTurtle : public Interactive {
	int Counter;
	void Move(GameState&) override;
	bool Die(Bunny*, ParticleExplosionType) override;
	void HitBy(GameObject&) override;
public:
	NormalTurtle(ObjectStartPos&);
};