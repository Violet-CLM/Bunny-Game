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
void Pickup::Behave(GameState& gameState)
{
	DetermineFrame(gameState.GameTicks >> 2);
	BounceYOffset = float(4 * sintable(int((gameState.GameTicks + /*obj.objectID * 8*/ + OriginX + PositionY * 256) * 16)));
}
void Pickup::HitBy(GameObject& other)
{
	if (other.ObjectType == BunnyObjectType::Player) {
		Explosion::AddExplosion(*this, ExplosionSetID, 86);
		Collected(static_cast<Bunny&>(other));
		//todo sound
		//todo points
		Delete();
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
	if (other.ObjectType == BunnyObjectType::Player && static_cast<Bunny&>(other).PlayerProperties.Ammo[AmmoID] < AMMO_MAX)
		Pickup::HitBy(other);
}
void AmmoPickup::Collected(Bunny& play) const
{
	int& ammoCounter = play.PlayerProperties.Ammo[AmmoID];
	if (ammoCounter == 0) play.fireType = AmmoID;
	ammoCounter = std::min(ammoCounter + (AmmoID != Weapon::Toaster) ? 3 : 96, AMMO_MAX);
}

void Gem::Draw(Layer* layers) const
{
	layers[SPRITELAYER].AppendSprite(mode, int(PositionX), int(PositionY + BounceYOffset), GetFrame(), DirectionX < 0);
}
void Gem::Collected(Bunny& play) const
{
	play.PlayerProperties.Gems[GemColor] += 1;
}

void Carrot::HitBy(GameObject& other)
{
	if (other.ObjectType == BunnyObjectType::Player && static_cast<Bunny&>(other).Health < START_HEALTH)
		Pickup::HitBy(other);
}
void Carrot::Collected(Bunny& play) const
{
	play.Health += 1;
}
void FullEnergy::HitBy(GameObject& other)
{
	if (other.ObjectType == BunnyObjectType::Player && static_cast<Bunny&>(other).Health < START_HEALTH)
		Pickup::HitBy(other);
}
void FullEnergy::Collected(Bunny& play) const
{
	play.Health = START_HEALTH;
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
