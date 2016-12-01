#include "Pickups.h"
#include "Game.h"
#include "BunnyMisc.h"
#include "BunnyEffects.h"
#include "BunnyWeapons.h"
#include "BunnyObjectList.h"
#include "BunnyVersionDependentStuff.h"

int Pickup::ExplosionSetID = 0;

Pickup::Pickup(ObjectStartPos & objStart, int ai) : BunnyObject(objStart) {
	AnimID = ai;
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
			//if ((SpeedY>4*65536) && ((obj->load==aSILVERCOIN) || (obj->load==aGOLDCOIN)))
				//PlaySample(PositionX,PositionY,sCOMMON_COIN,0,0);

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
			Explosion::AddExplosion(*this, ExplosionSetID, 86);
			Collected(static_cast<Bunny&>(other));
			//todo sound
			//todo points
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

AmmoPickup::AmmoPickup(ObjectStartPos & objStart, int ai) : Pickup(objStart, ai), AmmoID(ai), AnimIDNormal(AmmoIconAnimIDs[ai]), AnimIDPoweredUp(AmmoIconAnimIDs[ai]-1) {}
void AmmoPickup::Behave(GameState& gameState)
{
	AnimID = AnimIDNormal; //todo check player powerup status
	Pickup::Behave(gameState);
}
void AmmoPickup::HitBy(GameObject& other)
{
	if (!(other.ObjectType == BunnyObjectType::Player && static_cast<Bunny&>(other).PlayerProperties.Ammo[AmmoID] >= AMMO_MAX))
		Pickup::HitBy(other);
}
void AmmoPickup::Collected(Bunny& play) const
{
	int& ammoCounter = play.PlayerProperties.Ammo[AmmoID];
	if (ammoCounter == 0) play.fireType = AmmoID;
	ammoCounter = std::min(ammoCounter + (AmmoID != Weapon::Toaster) ? 3 : 96, AMMO_MAX);
}

void Gem::Draw(Layer* layers) const {
	DrawNormally(layers, mode);
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
