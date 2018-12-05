#include "Diamondus.h"
#include "BunnyMisc.h"
#include "BunnyEffects.h"
#include "BunnyObjectList.h"
#include "BunnyVersionDependentStuff.h"
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
		}
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


NormalTurtle::NormalTurtle(ObjectStartPos& start) : Interactive(start) {
	Energy = 1;
	DirectionX = 1;
}
void NormalTurtle::Move(GameState& gameState) {
	enum TurtleAnims { mTURTLE_ATTACK, mTURTLE_IDLE, mTURTLE_INSHELL, mTURTLE_OUTSHELL, mTURTLE_REVSHELL, mTURTLE_SHELL /*unused*/, mTURTLE_STAND /*unused*/, mTURTLE_WALK };

	switch (State) {
	case State::Start:
		AnimID = mTURTLE_WALK;
		State = State::Walk;
		PutOnGround(true);
		break;

	case State::Walk: {
		PositionX += DirectionX; //default speed

		int px, py;
		const auto frame = GetFrame();
		px = (frame.Width + frame.HotspotX) * DirectionX;
		py = frame.HotspotY - frame.ColdspotY - 16;

		Event tileAttr;
		bool hit1 = gameState.MaskedPixel(int(PositionX + px), int(PositionY + py) + 16);
		bool hit2 = gameState.MaskedPixel(int(PositionX + px), int(PositionY + py), tileAttr);

		if (hit2 || !hit1 || (tileAttr.ID == EventIDs::STOPENEMY)) { //bounce of wall
			if (gameState.MaskedPixel(int(PositionX - px), int(PositionY + py))) { //dont do the direction swap
				State = State::Wait;
			} else {
				PositionX -= DirectionX;
				State = State::Turn;
				Counter = 0;
				AnimID = mTURTLE_INSHELL;
				CausesRicochet = true;
				PlaySampleAtObject(Turtle, HIDE);
			}
		}

		AnimateIfZero(!!(gameState.GameTicks & 7));

		if (Counter > AISPEED) {
			int Distance = 128 * 128;
			const auto closestplayer = GetNearestPlayerRef(Distance);
			if (closestplayer != nullptr) {
				if (((DirectionX < 0) && (closestplayer->PositionX < PositionX)) ||
					((DirectionX > 0) && (closestplayer->PositionX > PositionX)) &&
					(abs(closestplayer->PositionY - PositionY) < 64))
				{
					if (Distance < 80 * 80)
						State = State::Attack;
					else
						if ((Distance > 90 * 90) && (closestplayer->lastFire < 20))
							State = State::Action;
					//else nothing: just walk ;-)
				}
			}
		}

		if (++Counter > AISPEED * 4) {
			State = State::Idle;
			Counter = 0;
			FrameID = 0;
			AnimID = mTURTLE_IDLE;
		}


		if ((Counter & 31) == 1) {
			const int randomSample = RandFac(7);
			if (randomSample < 4)
				PlaySampleAtObject(Turtle, SPK1TURT + randomSample);
		} //whatever

		break; }

	case State::Wait: //rare
		AnimateIfZero(!!(gameState.GameTicks & 7));
		break;

	case State::Idle:
		if ((++Counter & 7) == 7)
		{
			if (++FrameID == 2) {
				PlaySampleAtObject(Turtle, IDLE1 + RandFac(1));
			} else if (FrameID >= GetFrameCount()) {
				FrameID = 0;
				Counter = 0;
				AnimID = mTURTLE_WALK;
				State = State::Walk;
			}
		}
		break;


	case State::Turn: //turn cycle at walls
		if (Counter == 0)
			PlaySampleAtObject(Turtle, TURN);

		if ((++Counter & 7) == 7) {
			if ((++FrameID == 3) && (AnimID == mTURTLE_OUTSHELL)) {
				PlaySampleAtObject(Common, PLOP1 + RandFac(3));
			}

			if (FrameID >= GetFrameCount()) {
				FrameID = 0;
				Counter = 0;
				if (AnimID == mTURTLE_INSHELL) {
					DirectionX = -DirectionX;
					AnimID = mTURTLE_OUTSHELL;
				} else {
					State = State::Walk;
					AnimID = mTURTLE_WALK;
					CausesRicochet = false;
				}
			}
		}
		break; //turn cycle

	case State::Action:
		if (GetNearestPlayer(100 * 100) != nullptr) { //go into shell if player close, and STAY THERE!
			CausesRicochet = true;

			if (AnimID != mTURTLE_INSHELL) {
				AnimID = mTURTLE_INSHELL;
				FrameID = 0;
				PlaySampleAtObject(Turtle, HIDE);
			}

			if ((++Counter & 3) == 3) { //fast!
				FrameID += 1;
				while (FrameID >= GetFrameCount())
					FrameID -= 1; //wait it out!
			}
		} else { //get outta shell
			CausesRicochet = false;

			if (AnimID != mTURTLE_OUTSHELL) {
				AnimID = mTURTLE_OUTSHELL;
				FrameID = 0;
			}

			if ((++Counter & 3) == 3) { //fast!
				if (++FrameID >= GetFrameCount()) {
					FrameID = 0;
					Counter = 0;
					AnimID = mTURTLE_WALK;
					State = State::Walk;
				}
			}
		}
		break;

	case State::Attack:
		if (AnimID != mTURTLE_ATTACK) {
			AnimID = mTURTLE_ATTACK;
			FrameID = 0;
		}

		if ((++Counter & 3) == 3) { //fast!
			if (++FrameID >= GetFrameCount()) {
				FrameID = 0;
				AnimID = mTURTLE_WALK;
				State = State::Walk;
				Counter = 0;
			} else if (FrameID == 3)
				PlaySampleAtObject(Turtle, NECK);
			else if (FrameID == 6)
				PlaySampleAtObject(Turtle, BITE3);
		}
		break;
	}

	MakeRectangularCollisionShapeBasedOnCurrentFrame(); //turtles change their dimensions so much this is the easiest solution
}
bool NormalTurtle::Die() {
	const int explosionAnimIDs[] = { 7, 71 };
	for (int i = 0; i < 2; ++i) {
		auto& explosion = AddExplosion(AnimSets::Ammo, explosionAnimIDs[i]);
		explosion.PositionX += Rand2Fac(32767) / 4096;
		explosion.PositionY += Rand2Fac(32767) / 4096;
	}
	PlaySampleAtObject(Common, SWISH1 + RandFac(7));

	/*obj->ProcPtr = &turtleshell;

	AnimID = mTURTLE_REVSHELL;
	SpeedY = -4 * 65536;

	obj->objtype = oSPECIAL;
	obj->nohit = 3; //dont
	obj->energy = 2;

	State = State::Start;	//skip random randpart
	obj->points = 100;
	obj->load = aTURTLESHELL;*/

	return Interactive::Die();
}
void NormalTurtle::HitBy(GameObject& other) {
	if (!(CausesRicochet && other.ObjectType == BunnyObjectType::Player && static_cast<Bunny&>(other).GetAttackType(false) == Bunny::AttackTypes::NotAttacking)) //ignore non-violent collisions when not in shell
		Interactive::HitBy(other);
}