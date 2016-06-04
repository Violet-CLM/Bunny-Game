#include "Diamondus.h"
#include "BunnyMisc.h"

void Bee::Behave(GameState& gameState)
{
	++Counter;
	DetermineFrame(gameState.GameTicks / 6);

	PositionX = OriginX + sintable(Counter * 8) * 16;
	if ((Counter & 127) < 63)
		DirectionX = -1;
	else
		DirectionX = +1;
	PositionY = OriginY +
		costable(Counter * 8) * 16 +
		sintable(gameState.GameTicks * 8) * 4;
}
