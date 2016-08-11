#include "BunnyObject.h"

void BunnyObject::Draw(Layer* layers) const
{
	layers[SPRITELAYER].DrawSprite(SpriteMode::Paletted, int(PositionX), int(PositionY), GetFrame(), DirectionX < 0, DirectionY < 0);
}