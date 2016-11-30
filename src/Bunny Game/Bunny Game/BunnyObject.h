#pragma once
#include "Game.h"
#include "Level.h"
#include "Objects.h"

namespace BunnyObjectType {
	enum {
		NonInteractive = 0,
		Player, Pickup, Enemy, PlayerBullet, EnemyBullet
	};
}

class BunnyObject : public GameObject { //put deactivates, etc. code in here
	using GameObject::GameObject;
protected:
	void Draw(Layer*) const override;
	int DirectionX, DirectionY;
};