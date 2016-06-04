#pragma once
#include "Game.h"
#include "Level.h"
#include "Objects.h"

class BunnyObject : public GameObject { //put deactivates, etc. code in here
	using GameObject::GameObject;
	void Draw(Layer*) const override;
protected:
	int DirectionX, DirectionY;
};