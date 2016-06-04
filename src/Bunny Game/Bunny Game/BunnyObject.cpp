#include "BunnyObject.h"

void BunnyObject::Draw(Layer* layers) const
{
	GetFrame().Draw(layers[SPRITELAYER], SpriteMode::Paletted, int(PositionX), int(PositionY), DirectionX < 0, DirectionY < 0);
}