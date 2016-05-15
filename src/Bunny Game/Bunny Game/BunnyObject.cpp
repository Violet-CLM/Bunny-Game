#include "BunnyObject.h"

float sintable(int a) {
	return sin((a & 1023) * 0.00613592314f); //convert from 1024 to tau
}
float costable(int a) {
	return sintable(a + 256);
}

void BunnyObject::Draw(Layer* layers) const
{
	GetFrame().Draw(layers[SPRITELAYER], int(PositionX), int(PositionY), DirectionX < 0, DirectionY < 0);
}