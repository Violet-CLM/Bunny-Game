#include "Pickups.h"
#include "Lattice.h"
#include "BunnyMisc.h"
#include "BunnyEffects.h"
#include "BunnyWeapons.h"
#include "BunnyObjectList.h"
#include "BunnyVersionDependentStuff.h"

int Pickup::ExplosionSetID = 0;

Pickup::Pickup(ObjectStartPos & objStart, int ai, unsigned int p, int s) : BunnyObject(objStart) {
	AnimID = ai;
	Points = p;
	Sample = s;
	if (((int(OriginX) >> 5) & 1) != ((int(OriginY) >> 5) & 1)) //checkerboard
		DirectionX = -1;
	ObjectType = BunnyObjectType::Pickup;
	CollisionShapes.emplace_back(10);
}
bool Pickup::InMotion() const {
	return SpeedY || SpeedX;
}
void Pickup::Behave(GameState& gameState)
{
	DetermineFrame(gameState.GameTicks >> 2);
	LowerToZero(TimeTillCollectable);

	if (InMotion()) {
		BounceYOffset = 0;

		const auto& frame = GetFrame();
		const int px = frame.HotspotX;
		const int py = frame.HotspotY + frame.Height + 2;

		if (gameState.MaskedPixel(int(PositionX + SpeedX), int(PositionY + py)))	//stuck
			SpeedX = 0;
		else if (SpeedX < -0.125f)
			SpeedX += 0.125f;
		else if (SpeedX > 0.125f)
			SpeedX -= 0.125f;
		else
			SpeedX = 0;

		PositionX += SpeedX;
		PositionY += SpeedY;
		SpeedY += (false/*underwater*/) ? 0.03125f : 0.125f;

		if (gameState.MaskedPixel(int(PositionX + px), int(PositionY + py))) {
			if (SpeedY > 4.f && (AnimID == 84 || AnimID == 37)) //silver or gold coin
				PlaySampleAtObject(Common, COIN);

			SpeedY /= -2;

			if (abs(SpeedY) < 0.25f)
				SpeedY = 0;
			else
				PositionY += SpeedY;
		};
	} else {
		int bouncePhase = gameState.GameTicks /*+ obj.objectID * 8*/;
		//if (underwater)
			//bouncePhase = BounceYOffset*2 + (OriginX + PositionY * 256) * 16;
		//else
			bouncePhase = int((bouncePhase + OriginX + PositionY * 256) * 16);
		BounceYOffset = float(4 * sintable(bouncePhase));
	}
}
void Pickup::HitBy(GameObject& other)
{
	if (other.ObjectType == BunnyObjectType::Player) {
		if (TimeTillCollectable == 0) {
			AddExplosion(ExplosionSetID, 86);
			auto& play = static_cast<Bunny&>(other);
			Collected(play);
			if (Sample) {
				//if (Sample == Samples::sCommon_PICKUP1)
					//PickupSample(PositionX, PositionY); //todo
				//else
					HostLevel.StartSound(AnimSets::Common, Sample, sf::Vector2f(PositionX, PositionY)); //almost all pickup samples are in Common, which helpfully is the same number in both 1.23 and TSF
			}
			GivePoints(play, Points);
			Delete();
		} //else do nothing
	} else if (other.ObjectType == BunnyObjectType::PlayerBullet && !InMotion()) {
		const PlayerBullet& bullet = static_cast<PlayerBullet&>(other); //is this right for Ice? TNT?
		SpeedX = bullet.SpeedX / 2;
		SpeedY = bullet.SpeedY / 2 + 0.015625f; //add a tiny bit so that it isn't 0 (and therefore immobile) until it stops itself intentionally by landing on the ground
		DirectionX = bullet.SpeedX > 0 ? 1 : -1;
	}
}
void Pickup::Draw(Layer* layers) const
{
	layers[SPRITELAYER].AppendSprite(SpriteMode::Paletted, int(PositionX), int(PositionY + BounceYOffset), GetFrame(), DirectionX < 0);
}

AmmoPickup::AmmoPickup(ObjectStartPos & objStart, int ai) : Pickup(objStart, ai, 100, Samples::sCommon_PICKUPW1), AmmoID(ai), AnimIDNormal(AmmoIconAnimIDs[ai]), AnimIDPoweredUp(AmmoIconAnimIDs[ai]-1) {
	Shift = (AmmoID != Weapon::Toaster
#ifdef BETAPEPPER
		&& AmmoID != Weapon::Gun8
#endif
		) ? 0 : 5;
}
void AmmoPickup::Behave(GameState& gameState)
{
	AnimID = AnimIDNormal; //todo check player powerup status
	Pickup::Behave(gameState);
}
void AmmoPickup::HitBy(GameObject& other)
{
	if (!(other.ObjectType == BunnyObjectType::Player && static_cast<Bunny&>(other).PlayerProperties.Ammo[AmmoID] >= (AMMO_MAX << Shift)))
		Pickup::HitBy(other);
}
void AmmoPickup::Collected(Bunny& play) const
{
	int& ammoCounter = play.PlayerProperties.Ammo[AmmoID];
	if (ammoCounter == 0) play.fireType = AmmoID;
	ammoCounter = std::min(ammoCounter + (3 << Shift), AMMO_MAX << Shift);
}

void Gem::Draw(Layer* layers) const {
	layers[SPRITELAYER].AppendSprite(mode, int(PositionX), int(PositionY + BounceYOffset), GetFrame(), DirectionX < 0);
}
void Gem::Collected(Bunny& play) const
{
	play.PlayerProperties.Gems[GemColor] += 1;
}

void Carrot::HitBy(GameObject& other)
{
	if (!(other.ObjectType == BunnyObjectType::Player && static_cast<Bunny&>(other).Health >= START_HEALTH))
		Pickup::HitBy(other);
}
void Carrot::Collected(Bunny& play) const
{
	play.Health += 1;
	play.AddToInvincibilityDuration(AISPEED);
}
void FullEnergy::HitBy(GameObject& other)
{
	if (!(other.ObjectType == BunnyObjectType::Player && static_cast<Bunny&>(other).Health >= START_HEALTH))
		Pickup::HitBy(other);
}
void FullEnergy::Collected(Bunny& play) const
{
	play.Health = START_HEALTH;
	play.AddToInvincibilityDuration(AISPEED * 5);
	//don't use regular Pickup::Sample property, instead do a switch based on the player's character:
	switch (play.PlayerProperties.CharacterIndex) {
	case char2JAZZ:
		PlaySampleAtObject(JazzSounds, JUMMY);
		break;
	case char2SPAZ:
		PlaySampleAtObject(SpazSounds, HAPPY);
		break;
	default:
		PlaySampleAtObject(Common, EAT1 + RandFac(3));
		break;
	}
}

void ExtraLife::Collected(Bunny& play) const
{
	play.PlayerProperties.Lives += 1;
}
void FlyCarrot::Collected(Bunny& play) const
{
	play.fly = 1;
}
void Food::Collected(Bunny& play) const
{
	play.EatFood();
}
void FreezeEnemies::Collected(Bunny& play) const
{
	play.freeze = 3*AISPEED;
}
void GoldCoin::Collected(Bunny& play) const
{
	play.PlayerProperties.Coins += 5;
}
void Invincibility::Collected(Bunny& play) const
{
	play.invincibility = 15*AISPEED;
}
void SilverCoin::Collected(Bunny& play) const
{
	play.PlayerProperties.Coins += 1;
}
