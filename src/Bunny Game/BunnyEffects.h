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
class FlickerLight : public BunnyObject {
	void Draw(Layer*) const override;
public:
	FlickerLight(ObjectStartPos& objStart) : BunnyObject(objStart) { LightType = LightType::Flicker; LightRadius = 112; }
};