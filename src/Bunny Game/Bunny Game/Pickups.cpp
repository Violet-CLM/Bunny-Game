#include "Pickups.h"

void BunnyObject::Draw(Layer* layers) const
{
	BasicProperties->GetFrame().Draw(layers[SPRITELAYER], int(BasicProperties->PositionX), int(BasicProperties->PositionY), DirectionX < 0, DirectionY < 0);
}


float sintable(int a) {
	return sin((a & 1023) * 0.00613592314f); //convert from 1024 to tau
}
float costable(int a) {
	return sintable(a + 256);
}

void Pickup::Behave(Level& level)
{
	DetermineFrame(level.GameTicks >> 2);
	BounceYOffset = float(4 * sintable(int((level.GameTicks + /*obj.objectID * 8*/ + BasicProperties->OriginX + BasicProperties->PositionY * 256) * 16)));
}

void Pickup::Draw(Layer* layers) const
{
	BasicProperties->GetFrame().Draw(layers[SPRITELAYER], int(BasicProperties->PositionX), int(BasicProperties->PositionY + BounceYOffset), DirectionX < 0);
}

void AmmoPickup::Behave(Level& level)
{
	BasicProperties->AnimID = AnimID; //todo check player powerup status
	Pickup::Behave(level);
}

void Bee::Behave(Level& level)
{
	++Counter;
	DetermineFrame(level.GameTicks / 6);

	BasicProperties->PositionX = BasicProperties->OriginX + sintable(Counter * 8) * 16;
	if ((Counter & 127) < 63)
		DirectionX = -1;
	else
		DirectionX = +1;
	BasicProperties->PositionY = BasicProperties->OriginY +
		costable(Counter * 8) * 16 +
		sintable(level.GameTicks * 8) * 4;
}
