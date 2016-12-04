#pragma once
#include "Game.h"
#include "Level.h"
#include "Objects.h"

namespace BunnyObjectType {
	enum {
		NonInteractive = 0,
		Player, Pickup, Interactive, PlayerBullet, EnemyBullet
	};
}

class BunnyObject : public GameObject { //put deactivates, etc. code in here
	using GameObject::GameObject;
protected:
	void DrawNormally(Layer*, const SpriteMode& = SpriteMode::Paletted) const;
	void Draw(Layer*) const override;
	int DirectionX, DirectionY;
};
class Interactive : public BunnyObject {
protected:
	bool CancelSpecialAttacks = false;
	unsigned int Points = 0, JustHit = 0;
	int Energy = 1;
	virtual void Move(GameState&) {}
	void Behave(GameState&) override;
	virtual bool Hurt(unsigned int, bool);
	void HitBy(GameObject&) override;
	void Draw(Layer*) const override;
public:
	Interactive(ObjectStartPos&, bool=true);
	bool IsEnemy, TriggersTNT; //separate from IsEnemy for purposes of destruct scenery
};