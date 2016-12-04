#include "Bunny.h"
#include "BunnyEffects.h"
#include "BunnyWeapons.h"
#include "BunnyObjectList.h"

void Bunny::AddBullet() { //figure out position and angle
	if (fireType == Weapon::TNT) { //TNT needs to be placed at the same (horizontal) position as the player to allow TNT climbing
		AddObject(EventIDs::TNT, PositionX, PositionY);
	} else {
		float angle;
		sf::Vector2f position(PositionX, PositionY);
		const AnimFrame& frame = GetFrame();
		if ( fireDirection == 8 ) { //up
			angle = AUP; //handle antigrav later, to avoid confusing pepper spray
		} else { //not up
			if (DirectionX >= 0) {
				angle = ARIGHT;
				if (swim) {// == swim_SWIM) {
					if (SpeedY >= 2.f)
						angle = ADOWNRIGHT;
					else if (SpeedY <= -2.f)
						angle = AUPRIGHT;
				}
			} else {
				angle = ALEFT;
				if (swim) {// == swim_SWIM) {
					if (SpeedY >= 2.f)
						angle = ADOWNLEFT;
					else if (SpeedY <= -2.f)
						angle = AUPLEFT;
				}
			}
		}

		if ( angle == AUP ) {
			frame.MovePositionToGunSpotX(position.x, DirectionX < 0);
			//if (drawPlayerUpsideDown(play))
			//	position.y += 8;
			//else
				position.y -= 8;
		} else { //not up
			frame.MovePositionToGunSpotY(position.y, false/*drawPlayerUpsideDown(play)*/);
		}
		/*if ( fly < 0 ) { //airboard
			const int adjustYForAirboard = 8 * (1 - sinTable(GameTicks * 12));
			//if (!drawPlayerUpsideDown(play))
				yPos -= adjustYForAirboard;
			//else
				//yPos += adjustYForAirboard;
		}*/

		/*if (characterIndex == char2BIRDRED) { //birds fire double, naturally
			AddBullet(position, (angle < ADOWN) ? (angle + ARFOFFSET) : (angle - ARFOFFSET));
		}*/
		AddBullet(position, angle);
	}

	DepleteAmmo();
}
void Bunny::AddBullet(sf::Vector2f position, float targetAngle, bool directlyFromPlayer) {
	const bool powerup = PlayerProperties.Powerups[fireType];
	const bool replaceWithShield = (directlyFromPlayer && fireType == Weapon::Blaster && shieldTime > 0);

	/*if ((wp->bubbles || (replaceWithShield && shieldType == 1)) && position.y > GameGlobals->level.waterLevel )
		AddSingleBullet(targetAngle, position, aBUBBLE, 0, true, &loadObjects[aBUBBLE].behavior, directlyFromPlayer);
	else*/ { //not a bubble
		int eventID;
		if (replaceWithShield) {
			return; //todo
			/*switch (shieldType) {
				case 1: eventID = aPLAYERBULLETC1; break;
				case 2: eventID = aPLAYERBULLETC2; break;
				case 3: eventID = aPLAYERBULLETC3; break;
				case 4: eventID = aPLAYERLASER; break;
				default: eventID = aPLAYERBULLET2; //why?
			}*/
		} else {
			eventID = (powerup ? EventIDs::PLAYERBULLETP1 : EventIDs::PLAYERBULLET1) + fireType;
			if (fireType > Weapon::TNT)
				eventID -= 1;
		}

		if (directlyFromPlayer && fireType == Weapon::Gun8) {
			if ( fireDirection != 8 ) {
				if ( targetAngle == ARIGHT ) {
					if ( fireAngle <= AUPLEFT && fireAngle >= ADOWN )
						fireAngle = ARIGHT;
				} else if ( targetAngle == ALEFT ) {
					if ( fireAngle >= AUPRIGHT && fireAngle <= ADOWN )
						fireAngle = ALEFT;
				}
			}
			if (abs(fireAngle - targetAngle) > API) {
				if (fireAngle < targetAngle)
					fireAngle += AFULLCIRCLE;
				else
					targetAngle += AFULLCIRCLE;
			}
			if ( fireAngle < targetAngle - APEPPEROFFET/2 )
				fireAngle += APEPPEROFFET;
			else if (fireAngle > targetAngle + APEPPEROFFET/2)
				fireAngle -= APEPPEROFFET;
			if (fireAngle >= AFULLCIRCLE) fireAngle -= AFULLCIRCLE;
			else if (fireAngle < AUP) fireAngle += AFULLCIRCLE;
			targetAngle = fireAngle;
		}

		if (fireType == Weapon::RF) {
			/*if (directlyFromPlayer && (targetAngle == float(ARIGHT) || targetAngle == float(ALEFT)) && obj->behavior.isRFBullet(obj)) { //normal horizontal non-mouse-aimed RFs; use exact old speeds to avoid messing with jumps and kicks and things
				int xSpeed = obj->xSpeed;
				int pxSpeed = (xSpeed + (xSpeed * direction)) & 0xFFFFFF00;
				AddSingleBullet(position, xSpeed, -1, pxSpeed, false, eventID);
				if (powerup)
					AddSingleBullet(position, xSpeed, 0, pxSpeed, false, eventID);
				AddSingleBullet(position, xSpeed, 1, pxSpeed, false, eventID);
			} else {
				AddSingleBullet(targetAngle-ARFOFFSET, position, eventID, directlyFromPlayer);
				if (powerup)
					AddSingleBullet(targetAngle, position, eventID, directlyFromPlayer);
				AddSingleBullet(targetAngle+ARFOFFSET, position, eventID, directlyFromPlayer);
			}*/
		} else if (fireType == Weapon::Gun8) {
			int numberOfBulletsToFire = 2;
			do
				AddSingleBullet(targetAngle + ((int)RandFac(0x3F) - 32) / 320.f, position, eventID, directlyFromPlayer, ((3<<16) + (RandFac(31)<<12)) / 65536.f);
				while ( --numberOfBulletsToFire );
		} else {
			float speed = 0;
			if (fireType == Weapon::Ice && powerup) {
				AddSingleBullet((DirectionX >= 0) ? ARIGHT : ALEFT, position, eventID, directlyFromPlayer);
			} else if (directlyFromPlayer && fireType == Weapon::Toaster) {
				if ( fireDirection == 8 )
					position.y -= 8;
				speed += (99 - PlayerProperties.FireSpeed) / 32.f;
			}
			AddSingleBullet(targetAngle, position, eventID, directlyFromPlayer, speed);
		}
	}
	/*if ( SoundGlobals->soundEnabled && weaponProfile[fireType].sample ) {
		if (fireType == 7 && (wp->spread != wspNORMALORBBGUN || checkedFireball) && wp->spread != wspBBGUN && !replaceWithShield)
			PlaySample(position, sCOMMON_RINGGUN, 0, 0);
		else {
			int realFireType = fireType;
			fireType = (replaceWithShield) ? 0 : fireType;
			AddBulletSounds(playerID, position);
			fireType = realFireType;
		}
	}*/
}
void Bunny::AddSingleBullet(float targetAngle, sf::Vector2f position, EventID eventID, bool directlyFromPlayer, float xSpeed) {
	bool reduceLifetime = false;
	float pxSpeed = 0;

	if (directlyFromPlayer) { //otherwise don't apply added speed based on player's speed
		/*if (getPlayerVarSettingLocal(play, pvANTIGRAV)) {
			yMod = -yMod; //antigrav shoots down, of course
		}*/
		if (fireDirection != 8) { //left/right
			pxSpeed = SpeedX;
			if (fireType != Weapon::RF) {
				LimitTo(pxSpeed, 8);
			}
			//pxSpeed &= 0xFFFFFF00;
		} else if (targetAngle == AUP) {
			reduceLifetime = true;
		}
	}

	AddSingleBullet(targetAngle, position, eventID, xSpeed, pxSpeed, reduceLifetime);
}
void Bunny::AddSingleBullet(float targetAngle, sf::Vector2f position, EventID eventID, float xSpeed, float pxSpeed,  bool reduceLifetime) {
	static_cast<PlayerBullet&>(AddObject(eventID, position.x, position.y)).Aim(targetAngle, xSpeed, pxSpeed, reduceLifetime);
}

PlayerBullet::PlayerBullet(ObjectStartPos& objStart, Weapon::Weapon id, int counterTarget) : BunnyObject(objStart), ammoID(id), CounterMustBeAtLeastThisHighToDrawBullet(counterTarget) {
	ObjectType = BunnyObjectType::PlayerBullet;
}
bool PlayerBullet::Ricochet()
{
	if (lastRico > 4 && ricos < 8) {//ricochet
		PositionX -= SpeedX + pxSpeed; //to move it away
		SpeedX = -SpeedX;
		AccelerationX = -AccelerationX;
		DirectionX = -DirectionX;
		SpeedY = std::min(std::max(-SpeedY + Rand2Fac(0x7FFF) / 4096.f, -8.f), 8.f);

		//for (t=0;t<8+(int)RandFac(7);t++)
		//	AddParticleSparks(obj->xpos,obj->ypos,-obj->direction);
		///PlaySample(obj->xpos,obj->ypos,ricosamps[RandFac(3)],0,0);

		ricos += 1;
		lastRico = 0;
		return true;
	}
	return false;
}
void PlayerBullet::Behave(GameState& gameState) {
	Move(gameState);
	++lastRico;
}
void PlayerBullet::Aim(float targetAngle, float xSpeed, float pxSpeedNew, bool reduceLifetime) {
	if (reduceLifetime) {
		lifeTime <<= 1; lifeTime /= 3;
	}
	if (!xSpeed)
		xSpeed = SpeedX;
	pxSpeed = pxSpeedNew;

	float ySpeed = -cos(targetAngle) * xSpeed;
	xSpeed *= sin(targetAngle);
	
	const float xRatio = xSpeed / SpeedX;
	const float yRatio = ySpeed / SpeedX;
	AccelerationY =	AccelerationY *	abs(xRatio)	+ AccelerationX *	yRatio;
	SpeedY =		SpeedY *		abs(xRatio)	+ SpeedX *			yRatio;
	AccelerationX *= xRatio;
	SpeedX = xSpeed;
	
	/*if (obj->behavior == cSINEBULLET) { //powered-up ice needs special treatment :( diagnosing by behavior should be AS-safe, however
		if (ySpeed && xSpeed) obj->behavior = cBULLET; //fixfix. sine bullets can't handle diagonals.
		else if (!first) obj->var1 = 512; //cosmetic
	}*/
	DirectionX = (AccelerationX >= 0) ? 1 : -1;
	/*if (obj->upAnim && !(obj->behavior == cBULLET) && abs(ySpeed) > abs(xSpeed) && !obj->behavior.isRFBullet(obj))
		obj->curAnim = obj->upAnim;
	else if (nuDirection == 0)
		nuDirection = 1; //toaster has very specific requirements for .direction*/
}
void PlayerBullet::Explode()
{
	Explosion::AddExplosion(*this, 0, killAnimID);
	//todo transfer lighttype
	Delete();
}
void PlayerBullet::Draw(Layer* layer) const {
	if (Counter >= CounterMustBeAtLeastThisHighToDrawBullet) //DONT DISPLAY THE BULLET WHEN STILL INSIDE CHARACTER!!
		DrawNormally(layer);
}


BlasterBullet::BlasterBullet(ObjectStartPos& objStart, bool poweredUp) : PlayerBullet(objStart, Weapon::Blaster, 3) {
	AnimID = poweredUp ? 20 : 17;
	CollisionShapes.emplace_back(11 + poweredUp*2, 4);
	SpeedX = float(6 + poweredUp);
	AccelerationX = poweredUp ? 0.1875f : 0.125f;
	killAnimID = 80;
	damage = 1 + poweredUp;
	lifeTime = AISPEED / 2 - poweredUp*5;
	//obj->lighttype = 2 - poweredUp;
}
void BlasterBullet::Move(GameState& gameState) {
	Event eventAtPixel;
	if (Counter > lifeTime)
		Explode();
	else if (Counter > 0 && gameState.MaskedPixel(int(PositionX), int(PositionY), eventAtPixel) && (eventAtPixel.ID != EventIDs::RICOCHET || !Ricochet())) {
		Explode();
	} else {
		++Counter;

		PositionX += SpeedX + pxSpeed;
		PositionY += SpeedY;

		SpeedX += AccelerationX;
		SpeedY += AccelerationY;

		ApproachZeroByUnit(pxSpeed, 0.125f);
		LimitTo(SpeedX, 10);

		DetermineFrame(gameState.GameTicks >> 2);
	}
}
void BlasterBullet::Draw(Layer* layer) const {
	if (Counter < CounterMustBeAtLeastThisHighToDrawBullet)
		return;
	//const auto angle = atan2(SpeedX, SpeedY);
	PlayerBullet::Draw(layer); //todo
}

BouncerBullet::BouncerBullet(ObjectStartPos& objStart) : PlayerBullet(objStart, Weapon::Bouncer, 7) {
	AnimID = 23;
	CollisionShapes.emplace_back(4);
	SpeedX = 5;
	SpeedY = 1;
	AccelerationX = 0.25f;
	AccelerationY = 0.0915527344f;
	killAnimID = 4;
	damage = 1;
	lifeTime = AISPEED * 3 / 2;
	//obj->lighttype = 2;
}
void BouncerBullet::Move(GameState& gameState) {
	PositionX += SpeedX + pxSpeed;
	PositionY += SpeedY;

	SpeedX += AccelerationX;
	//if (PositionY>level.waterlevel)
		//SpeedY+=AccelerationY+level.gravity/4;
	//else
		SpeedY += AccelerationY + 0.125f;//level.gravity;

	ApproachZeroByUnit(pxSpeed, 0.125f);
	LimitTo(SpeedX, 8);
	LimitTo(SpeedY, 4);

	int sample = 0;
	if (
		(SpeedY<0 && gameState.MaskedPixel(int(PositionX), int(PositionY - 4))) ||
		(SpeedY > 0 && gameState.MaskedPixel(int(PositionX), int(PositionY + 4)) && (PositionY -= SpeedY, true)) //only move up if bouncing off the floor
	) {
		++bounces;
		SpeedY = SpeedY * -7 / 8;
		sample += 1;
	}
	if (
		(SpeedX < 0 && gameState.MaskedPixel(int(PositionX - 2), int(PositionY))) ||
		(SpeedX > 0 && gameState.MaskedPixel(int(PositionX + 2), int(PositionY)))
	) {
		++bounces;
		SpeedX = SpeedX * -7 / 8;
		AccelerationX = -AccelerationX;
		sample += 1;
	}
	if (sample)
		;//PlaySample(PositionX,PositionY,sCOMMON_BLOKPLOP,40,0); //todo sample

	if (Counter++ > lifeTime || bounces > 16) {
		//lighttype=0;
		Explode();
	};
	
	DetermineFrame(gameState.GameTicks >> 2);
}

BouncerBulletPU::BouncerBulletPU(ObjectStartPos& objStart) : PlayerBullet(objStart, Weapon::Bouncer) {
	AnimID = 26;
	CollisionShapes.emplace_back(5);
	SpeedX = 5;						//these are all the same as for non-PU, curiously
	SpeedY = 1;						//
	AccelerationX = 0.25f;			//
	AccelerationY = 0.0915527344f;	//
	killAnimID = 4;					//
	lifeTime = AISPEED * 3 / 2;		//
	//obj->lighttype = 2;			//
	damage = 2;
}
void BouncerBulletPU::Move(GameState& gameState) {
	PositionX += SpeedX + pxSpeed;
	PositionY += SpeedY;
	
	SpeedX += AccelerationX;
	//if (PositionY>GameGlobals->level.waterLevel)
		//SpeedY+=AccelerationY+GameGlobals->level.gravity/4;
	//else
		SpeedY += AccelerationY + 0.125f;//GameGlobals->level.gravity;
	
	ApproachZeroByUnit(pxSpeed, 0.125f);
	LimitTo(SpeedX, 6);
	LimitTo(SpeedY, 6);

	bool sample = false;
	if (SpeedY < 0 && gameState.MaskedPixel(int(PositionX), int(PositionY - 4))) { //up
		++bounces;
		SpeedY = -(SpeedY * (9 + RandFac(7))) / 8;
		sample = true;
	} else if (SpeedY > 0 && gameState.MaskedPixel(int(PositionX), int(PositionY + 4))) { //down
		++bounces;
		SpeedY = SpeedY * -9 / 8;
		sample = true;
	}

	if (
		(SpeedX < 0 && gameState.MaskedPixel(int(PositionX - 2), int(PositionY))) ||//left
		(SpeedX > 0 && gameState.MaskedPixel(int(PositionX + 2), int(PositionY))) //right
	) {
		++bounces;
		SpeedX = SpeedX * -9 / 8;
		if (!RandFac(3))
			AccelerationX = -AccelerationX;
		sample = true;
	}

	if (sample)
		;//PlaySample(PositionX,PositionY,sCOMMON_BLOKPLOP,0,0);

	if (Counter++ > lifeTime || bounces > 16) {
		//lighttype=0;
		Explode();
	};
	
	DetermineFrame(gameState.GameTicks >> 2);
}

PepperSprayBullet::PepperSprayBullet(ObjectStartPos& start, bool poweredUp) : PlayerBullet(start, Weapon::Gun8) {
	AnimID = 9 + poweredUp;
	CollisionShapes.emplace_back(2 + poweredUp*4);
	SpeedX = 1;
	killAnimID = 10;
	damage = 1 + poweredUp;
	//if (poweredUp)
		//obj->lighttype = 2;
}
void PepperSprayBullet::Move(GameState& gameState) {
	if (!adjustedSpeedsPostAiming) {
		SpeedX += pxSpeed;
		adjustedSpeedsPostAiming = true;
	}
	PositionX += (SpeedX += AccelerationX) + pxSpeed;
	PositionY += SpeedY += AccelerationY;
	
	ApproachZeroByUnit(pxSpeed, 0.125f);

	if (++Counter > (AISPEED - 16) || gameState.MaskedPixel(int(PositionX), int(PositionY))) { //54 is hardcoded: shooting "up" does not reduce pepper spray's lifetime
		Explode();
		//light=2;
		//lighttype=1;
	}
}


ToasterBullet::ToasterBullet(ObjectStartPos& start, bool poweredUp) : PlayerBullet(start, Weapon::Toaster, 9) {
	AnimID = poweredUp ? 58 : 55;
	CollisionShapes.emplace_back(9);
	SpeedX = 4;
	AccelerationX = 0.125f;
	damage = 1 + poweredUp;
	lifeTime = AISPEED / 2;
}
void ToasterBullet::Explode() {
	if (damage == 1) //not powered up
		Delete();
	else {
		AnimID = FrameID = 0;
		NumberOfTimesToShowEndAnimation = 2; //number of times to do animation... it's possible in the original code for this to be 1 instead of 2, but only on rare occasion, so meh.
		ObjectType = BunnyObjectType::NonInteractive;
	}
}
void ToasterBullet::Move(GameState& gameState) {
	if (!Started) {
		Started = true;
		DistanceFromParentY = Parent->PositionY - PositionY;

		const float randomSpeed = Rand2Fac(3) * 0.015625f;
		const auto angle = (SpeedY != 0.f) ? 0 : atan2(-SpeedY, SpeedX); //among various changes for better spreading when fired at non-orthogonal angles
		AccelerationX = sin(angle) * randomSpeed;
		AccelerationY = cos(angle) * randomSpeed;

		return;
	}
	if (ObjectType == BunnyObjectType::NonInteractive) { //exploding after hitting a wall/Interactive
		//if (RandFac(7)==0)
		//	AddParticlePixel(obj->xpos,obj->ypos-65536*4,3);
		
		if ((gameState.GameTicks & 7) == 0 && (++FrameID >= GetFrameCount())) {
			if (--NumberOfTimesToShowEndAnimation <= 0)
				Delete();
			else
				FrameID = 0;
		}
		return;
	}

	if (abs(SpeedX) > abs(SpeedY)) {
		Minimize(SpeedX, 6);
		else Maximize(SpeedX, -6);
		else ApproachZeroByUnit(SpeedX, 0.015625f);
	} else {
		ApproachZeroByUnit(SpeedY, 0.015625f);
	}
	PositionX += (SpeedX += AccelerationX) + pxSpeed;
	PositionY += (SpeedY += AccelerationY);

	if (gameState.MaskedPixel(int(PositionX), int(PositionY))) {
		Explode();
	} else { //animate
		if ((Counter & 7) == 0 && ++FrameID >= GetFrameCount())
			Delete();
		else if (++Counter >= CounterMustBeAtLeastThisHighToDrawBullet)
			;//lightType = 1;
		else //in early moments, adjust PositionY to follow shooter's
			PositionY = Parent->PositionY - DistanceFromParentY;
	}
}

TNTBullet::TNTBullet(ObjectStartPos& start) : Interactive(start, false) {
	AnimID = 59;
	CollisionShapes.emplace_back(11);

	//lightType = 1;
}
bool TNTBullet::Hurt(unsigned int, bool fromBullet) {
	if (fromBullet) {
		ObjectType = BunnyObjectType::NonInteractive;
		Counter = FrameID = 0;
		return true;
	}
	return true;
}
void TNTBullet::Behave(GameState& gameState) {
	if (ObjectType == BunnyObjectType::Interactive) { //hasn't started exploding yet
		if (++Counter > 255)
			Hurt(1, true);
		else if (!(Counter & 3)) {
		//show counting animation
			DetermineFrame(FrameID + 1);

		//check for near objects!
			for (const auto& it : HostLevelObjectList) {
				if (it->ObjectType == BunnyObjectType::Interactive && static_cast<const Interactive&>(*it.get()).TriggersTNT) {
					const auto dx = PositionX - it->PositionX;
					if (dx > -64 && dx < 64) {
						const auto dy = PositionY - it->PositionY;
						if (dy > -64 && dy < 64) {
							Hurt(1, true);
							break; //nomore
						}	//dy
					}	//dx
				}	//for each active gameobj
			}
		}	//if objcounter&3==0
	} else {	//exploding
		if (++Counter < 20) {
			/*if (Counter==1) {
				if (RandFac(1))
					obj->var1=sCOMMON_BELL_FIRE;
				else
					obj->var1=sCOMMON_BELL_FIRE2;
			}	//counter=1*/

			//obj->channel=PlayLoopSample(obj->xpos,obj->ypos,obj->var1,0,0,obj->channel);
		} else if (Counter == 20) {
			//PlaySample(obj->xpos,obj->ypos,sCOMMON_EXPL_TNT,128,0);

			Explosion::AddExplosion(*this, 0, 77);
			//DoFullBlast(num,obj->xpos,obj->ypos,obj->creator,96*96);
				
			//obj->lighttype=5;
			light=18;
		} else {
			if (--light < 2) 
				Delete();
		}	//light
	}
}
void TNTBullet::Draw(Layer* layers) const {
	if (ObjectType == BunnyObjectType::Interactive)
		DrawNormally(layers);
	else if (Counter < 20) {
		const float scale = (((256 + (Counter-50) * (Counter-50)) / 4) & 255) / 32.f;
		layers[SPRITELAYER].AppendResizedSprite(SpriteMode::Paletted, int(PositionX), int(PositionY), GetFrame(), scale,scale);
	} //else don't draw, let the Explosion object handle that
}
