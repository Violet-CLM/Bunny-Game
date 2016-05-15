#pragma once
#include "Game.h"
#include "Level.h"
#include "Objects.h"

float sintable(int);
float costable(int);

class BunnyObject : public GameObject { //put deactivates, etc. code in here
	using GameObject::GameObject;
	void Draw(Layer*) const override;
protected:
	int DirectionX, DirectionY;
};