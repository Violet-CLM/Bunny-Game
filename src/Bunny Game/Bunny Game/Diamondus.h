#pragma once
#include "BunnyObject.h"

class Bee : public Enemy {
	int Counter;
	void Move(GameState&) override;
public:
	Bee(ObjectStartPos&);
};