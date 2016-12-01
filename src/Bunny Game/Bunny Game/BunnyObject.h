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
	void DrawNormally(Layer*, const SpriteMode& = SpriteMode::Paletted) const;
	void Draw(Layer*) const override;
	int DirectionX, DirectionY;
};
class Enemy : public BunnyObject {
protected:
	bool CancelSpecialAttacks = false;
	unsigned int Points = 0, JustHit = 0;
	int Energy = 1;
	virtual void Move(GameState&) = 0;
	void Behave(GameState&) override;
	bool Hurt(unsigned int, bool);
	void HitBy(GameObject&) override;
	void Draw(Layer*) const override;
public:
	Enemy(ObjectStartPos&);
};