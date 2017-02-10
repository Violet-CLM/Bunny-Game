#pragma once
#include "BunnyObject.h"

class Explosion : public BunnyObject {
	int counter;

	using BunnyObject::BunnyObject;
	void Behave(GameState&) override;
};

class SteadyLight : public BunnyObject {
	void Draw(Layer*) const override { DrawObjectToLightBuffer(*this); }
public:
	SteadyLight(ObjectStartPos& objStart) : BunnyObject(objStart) { MakeNormal(20); }
};