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
	int lifeTime, damage, killAnimID, lastRico = 0, ricos = 0;
	float SpeedX, SpeedY = 0, AccelerationX, AccelerationY = 0, pxSpeed;

	bool Ricochet();
	virtual void Move(GameState&) = 0;
	void Behave(GameState&) override;
	//void HitBy(GameObject&) override;
public:
	PlayerBullet(ObjectStartPos&, Weapon::Weapon);
	void Aim(float targetAngle, float xSpeed, float pxSpeed, bool reduceLifetime);
	void Explode();
};

class BlasterBullet : public PlayerBullet {
	int counter = 0;
	void Move(GameState&) override;
	void Draw(Layer*) const override;
public:
	BlasterBullet(ObjectStartPos&, bool);
};