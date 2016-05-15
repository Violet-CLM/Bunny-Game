#pragma once
#include "BunnyObject.h"

class Bee : public BunnyObject {
	using BunnyObject::BunnyObject;
	int Counter;
	void Behave(GameState&) override;
};