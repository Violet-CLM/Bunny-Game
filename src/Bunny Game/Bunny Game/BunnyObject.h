#pragma once
#include "Lattice.h"
#include "Level.h"
#include "Objects.h"

namespace BunnyObjectType {
	enum {
		NonInteractive = 0,
		Player, Pickup, Interactive, PlayerBullet, EnemyBullet
	};
}

class Explosion;
class BunnyObject : public GameObject { //put deactivates, etc. code in here
	using GameObject::GameObject;
protected:
	void DrawNormally(Layer*, const SpriteMode& = SpriteMode::Paletted) const;
	void Draw(Layer*) const override;
	void DoBlast(int, bool=false);
	int DirectionX, DirectionY;
	Explosion& AddExplosion(int setID, int animID, bool makeChild = false);
};
class Interactive : public BunnyObject {
protected:
	bool CancelSpecialAttacks = false;
	unsigned int Points = 0, JustHit = 0;
	int Energy = 1;
	virtual void Move(GameState&) {}
	void Behave(GameState&) override;
	void HitBy(GameObject&) override;
	void Draw(Layer*) const override;
public:
	Interactive(ObjectStartPos&, bool=true);
	virtual bool Hurt(unsigned int, bool);
	bool IsEnemy, TriggersTNT; //separate from IsEnemy for purposes of destruct scenery
};