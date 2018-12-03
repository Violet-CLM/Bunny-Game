#include "Diamondus.h"
#include "BunnyMisc.h"
#include "Bunny.h"

Bee::Bee(ObjectStartPos& start) : Interactive(start) {
	CollisionShapes.emplace_back(24,33);
	Energy = 1;
	State = State::Idle;
}
void Bee::Move(GameState& gameState) {
	DetermineFrame(gameState.GameTicks / 6);
	switch (State) {
	case State::Idle:
		++Counter;

		PositionX = OriginX + sintable(Counter * 8) * 16;
		if ((Counter & 127) < 63)
			DirectionX = -1;
		else
			DirectionX = +1;
		PositionY = OriginY +
			costable(Counter * 8) * 16 +
			sintable(gameState.GameTicks * 8) * 4;

		Target = GetNearestPlayer(256 * 256);
		if (Target != nullptr) {
			DirectionX = PositionX < Target->PositionX ? 1 : -1;
			State = State::Attack;
			Counter = AISPEED * 2;
		}
		break;
	case State::Attack: {
		auto wantsx = Target->PositionX - PositionX;
		auto wantsy = Target->PositionY - PositionY;

		auto maxspeed = float(2 + 1);// level.difficulty); //todo difficulty
		LimitTo(wantsx, maxspeed);
		LimitTo(wantsy, maxspeed);

		ApproachConstantByConstant(ApproachSpeedX, wantsx, 0.0625);
		ApproachConstantByConstant(ApproachSpeedY, wantsy, 0.0625);

		PositionX += ApproachSpeedX;
		PositionY += ApproachSpeedY;
		DirectionX = int(ApproachSpeedX);

		if (--Counter == 0) {
			State = State::Fly;	//away
			Counter = 64; //number of ticks to spend retreating
			RetreatSpeedX = (OriginX - PositionX) / Counter;
			RetreatSpeedY = (OriginY - PositionY) / Counter;
			DirectionX = (RetreatSpeedX < -0.5) ? -1 : 1;
		};
		break; }

	case State::Fly: //away
		PositionX += RetreatSpeedX;
		PositionY += RetreatSpeedY;

		if (--Counter == 0)
			State = State::Idle;
		break;
	}

	//todo water
	//todo loopsample
}

