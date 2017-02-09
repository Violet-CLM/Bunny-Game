#pragma once
#include "BunnyObject.h"

class Explosion : public BunnyObject {
	int counter;

	using BunnyObject::BunnyObject;
	void Behave(GameState&) override;
};
