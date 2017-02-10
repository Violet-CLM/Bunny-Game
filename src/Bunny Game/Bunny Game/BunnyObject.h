#pragma once
#include "Lattice.h"
#include "Level.h"
#include "Objects.h"
#include "PostProcessing.h"

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
	void Behave(GameState&) override;
	void DoBlast(int, bool=false);
	int DirectionX, DirectionY;
	Explosion& AddExplosion(int setID, int animID, bool makeChild = false);
public:
	LightType::LightType LightType; LightParam LightIntensity, LightRadius;
protected: //series of convenience methods for turning JJ2 constants into their composite values
	void MakePoint1() { LightType = LightType::Point; LightRadius = 20; LightIntensity = NormalIntensity >> 3; }
	void MakePoint2() { LightType = LightType::Point; LightRadius = 20; LightIntensity = NormalIntensity >> 2; }
	void MakeNormal(LightParam brightness) { LightType = LightType::Normal; LightRadius = brightness * 4 + 16; LightIntensity = NormalIntensity; }
	void MakeBright(LightParam brightness) { LightType = LightType::Normal; LightRadius = brightness * 4 + 16; LightIntensity = 2 * brightness + 60; }
	void MakeRing1(LightParam brightness) { LightType = LightType::Ring; LightRadius = brightness * 4 + 8; LightIntensity = 40 - brightness; } //only + 8, not + 16, because the fade size is smaller
	void MakeRing2(LightParam brightness) { LightType = LightType::Ring; LightRadius = brightness * 4 + 8; LightIntensity = 140 - brightness; }
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