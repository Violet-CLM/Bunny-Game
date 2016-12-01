#pragma once

#define AMMO_MAX 99

#define API (3.14159265359f)
#define AUP (0.f)
#define AUPRIGHT (0.25f * API)
#define ARIGHT (0.5f * API)
#define ADOWNRIGHT (0.75f * API)
#define ADOWN (API)
#define ADOWNLEFT (1.25f * API)
#define ALEFT (1.5f * API)
#define AUPLEFT (1.75f * API)
#define AFULLCIRCLE (2.f * API)

#define ARFOFFSET (0.064f * API)
#define APEPPEROFFET (0.125f * API)

namespace Weapon {
	enum Weapon {
		Blaster, Bouncer, Ice, Seeker, RF, Toaster, TNT, Gun8, Gun9
		, LAST
	};
}

const int AmmoIconAnimIDs[Weapon::LAST] = {-1, 25, 29, 34, 49, 57, 59, 62, 68};

class PlayerBullet : public BunnyObject {
protected:
	Weapon::Weapon ammoID;
	int Counter = 0, CounterMustBeAtLeastThisHighToDrawBullet;
	int lifeTime, killAnimID, lastRico = 0, ricos = 0;
	float AccelerationX, AccelerationY = 0, pxSpeed;

	bool Ricochet();
	virtual void Move(GameState&) = 0;
	void Behave(GameState&) override;
	void Draw(Layer*) const override;
public:
	PlayerBullet(ObjectStartPos&, Weapon::Weapon, int = 0);
	void Aim(float targetAngle, float xSpeed, float pxSpeed, bool reduceLifetime);
	virtual void Explode();

	int damage;
	float SpeedX, SpeedY = 0;
};

class BlasterBullet : public PlayerBullet {
	int counter = 0;
	void Move(GameState&) override;
	void Draw(Layer*) const override;
public:
	BlasterBullet(ObjectStartPos&, bool);
};

class BouncerBullet : public PlayerBullet {
	int counter = 0, bounces = 0;
	void Move(GameState&) override;
public:
	BouncerBullet(ObjectStartPos&);
};

class BouncerBulletPU : public PlayerBullet {
	int counter = 0, bounces = 0;
	void Move(GameState&) override;
public:
	BouncerBulletPU(ObjectStartPos&);
};

class ToasterBullet : public PlayerBullet {
	int counter = 0;
	float DistanceFromParentY = 0;
	bool Started = false;
	void Move(GameState&) override;
public:
	void Explode() override;
	ToasterBullet(ObjectStartPos&, bool);
};

class PepperSprayBullet : public PlayerBullet {
	bool adjustedSpeedsPostAiming = false;
	int counter = 0;
	void Move(GameState&) override;
public:
	PepperSprayBullet(ObjectStartPos&, bool);
};
