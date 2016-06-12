#include "Pickups.h"
#include "Game.h"
#include "BunnyMisc.h"

Pickup::Pickup(ObjectStartPos & objStart, int ai) : BunnyObject(objStart) {
	AnimID = ai;
	if (((int(OriginX) >> 5) & 1) != ((int(OriginY) >> 5) & 1)) //checkerboard
		DirectionX = -1;
	ObjectType = BunnyObjectType::Pickup;
	RadiusX = RadiusY = 8;
	RoundedCorners = false;//true;
}
void Pickup::Behave(GameState& gameState)
{
	DetermineFrame(gameState.GameTicks >> 2);
	BounceYOffset = float(4 * sintable(int((gameState.GameTicks + /*obj.objectID * 8*/ + OriginX + PositionY * 256) * 16)));
}
void Pickup::HitBy(GameObject& other)
{
	if (other.ObjectType == BunnyObjectType::Player)
		Delete();
}
void Pickup::Draw(Layer* layers) const
{
	GetFrame().Draw(layers[SPRITELAYER], SpriteMode::Paletted, int(PositionX), int(PositionY + BounceYOffset), DirectionX < 0);
}

AmmoPickup::AmmoPickup(ObjectStartPos & objStart, int ai, int an, int anp) : Pickup(objStart, ai), AmmoID(ai), AnimIDNormal(an), AnimIDPoweredUp(anp) {}
void AmmoPickup::Behave(GameState& gameState)
{
	AnimID = AnimIDNormal; //todo check player powerup status
	Pickup::Behave(gameState);
}

void Gem::Draw(Layer* layers) const
{
	GetFrame().Draw(layers[SPRITELAYER], mode, int(PositionX), int(PositionY + BounceYOffset), DirectionX < 0);
}
