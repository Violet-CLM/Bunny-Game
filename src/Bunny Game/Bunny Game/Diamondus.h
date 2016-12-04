#pragma once
#include "BunnyObject.h"

class Bee : public Interactive {
	int Counter;
	void Move(GameState&) override;
public:
	Bee(ObjectStartPos&);
};