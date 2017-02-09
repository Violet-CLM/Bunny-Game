#include "Bunny.h"
#include "BunnyEffects.h"
#include "BunnyWeapons.h"
#include "BunnyObjectList.h"
#include "BunnyVersionDependentStuff.h"

void Bunny::AddBullet() { //figure out position and angle
	if (fireType == Weapon::TNT) { //TNT needs to be placed at the same (horizontal) position as the player to allow TNT climbing
		AddObject(EventIDs::TNT, PositionX, PositionY, true);
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
			if (directlyFromPlayer && (targetAngle == float(ARIGHT) || targetAngle == float(ALEFT))) { //normal horizontal non-mouse-aimed RFs; use exact old speeds to avoid messing with jumps and kicks and things
				const bool right = targetAngle == float(ARIGHT);
				RFBullet::AddHorizontalRFBullet(*this, position, SpeedX, -1, powerup,right);
				if (powerup)
					RFBullet::AddHorizontalRFBullet(*this, position, SpeedX, 0, powerup,right);
				RFBullet::AddHorizontalRFBullet(*this, position, SpeedX, 1, powerup,right);
			} else {
				AddSingleBullet(targetAngle-ARFOFFSET, position, eventID, directlyFromPlayer);
				if (powerup)
					AddSingleBullet(targetAngle, position, eventID, directlyFromPlayer);
				AddSingleBullet(targetAngle+ARFOFFSET, position, eventID, directlyFromPlayer);
			}
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

		switch (fireType) {
		case Weapon::Blaster:
			if (!powerup) {
				if (PlayerProperties.CharacterIndex == char2JAZZ)
					PlaySample(Ammo, GUNJAZZ, position, 0, 22050);
				else
					PlaySample(Ammo, GUN1 + (RandFac(255) % 3), position, 0, 22050);
			} else {
				if (PlayerProperties.CharacterIndex == char2JAZZ)
					PlaySample(Ammo, FUMP, position, 0, 22050);
				else
					PlaySample(Ammo, FUMP + (RandFac(1) * 3), position);
			}
			break;
		case Weapon::Bouncer:
			PlaySample(Ammo, BMP1 + (RandFac(255) % 6), position, 0, (PlayerProperties.CharacterIndex == char2JAZZ) ? 11025 : 22050);
			break;
		case Weapon::Ice:
			if (powerup)
				PlaySample(Ammo, ICEPU1 + RandFac(3), position);
			else
				PlaySample(Ammo, ICEGUN + (RandFac(255) % 3), position);
			break;
		case Weapon::Seeker:
			PlaySample(Ammo, MISSILE, position);
			break;
		case Weapon::RF:
			PlaySample(Ammo, LAZRAYS, position);
			break;
		case Weapon::Toaster:
			break;
		case Weapon::TNT:
			break;
		case Weapon::Gun8:
			PlaySample(Ammo, GUNVELOCITY, position);
			break;
		case Weapon::Gun9:
			PlaySample(Ammo, LASER2 + RandFac(1), position);
			break;
		}
	}

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
	static_cast<PlayerBullet&>(AddObject(eventID, position.x, position.y, true)).Aim(targetAngle, xSpeed, pxSpeed, reduceLifetime);
}

PlayerBullet::PlayerBullet(ObjectStartPos& objStart, Weapon::Weapon id, int counterTarget, bool angle) : BunnyObject(objStart), ammoID(id), CounterMustBeAtLeastThisHighToDrawBullet(counterTarget), DrawAtAngle(angle) {
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
		PlaySampleAtObject(Ammo, BUL1 + RandFac(3));

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
}
void PlayerBullet::Explode()
{
	Explosion explosion = AddExplosion(0, killAnimID);
	explosion.LightType = LightType;
	explosion.LightIntensity = LightIntensity;
	explosion.LightRadius = LightRadius;
	Delete();
}
void PlayerBullet::Draw(Layer* layers) const {
	if (Counter >= CounterMustBeAtLeastThisHighToDrawBullet) { //DONT DISPLAY THE BULLET WHEN STILL INSIDE CHARACTER!!
		if (!DrawAtAngle)
			DrawNormally(layers);
		else
			layers[SPRITELAYER].AppendRotatedSprite(SpriteMode::Paletted, int(PositionX), int(PositionY), GetFrame(), atan2((SpeedX < 0) ? SpeedY : -SpeedY, abs(SpeedX)), float(DirectionX));
	}
	if (LightType != LightType::None)
		DrawObjectToLightBuffer(*this);
}


BlasterBullet::BlasterBullet(ObjectStartPos& objStart, bool poweredUp) : PlayerBullet(objStart, Weapon::Blaster, 3, true) {
	AnimID = poweredUp ? 20 : 17;
	CollisionShapes.emplace_back(11 + poweredUp*2, 4);
	SpeedX = float(6 + poweredUp);
	AccelerationX = poweredUp ? 0.1875f : 0.125f;
	killAnimID = 80;
	damage = 1 + poweredUp;
	lifeTime = AISPEED / 2 - poweredUp*5;
	if (poweredUp)
		MakePoint1();
	else
		MakePoint2();
}
void BlasterBullet::Move(GameState& gameState) {
	Event eventAtPixel;
	if ((Counter > lifeTime) || (Counter > 0 && gameState.MaskedPixel(int(PositionX), int(PositionY), eventAtPixel) && (eventAtPixel.ID != EventIDs::RICOCHET || !Ricochet()))) {
		PlaySample(Common, EXPSM1, sf::Vector2f(PositionX,PositionY), 80); //separate from Explode because blaster bullets don't make a sound when they die from hitting an enemy
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
	MakePoint2();
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
		PlaySample(Common, BLOKPLOP, sf::Vector2f(PositionX,PositionY), 40);

	if (Counter++ > lifeTime || bounces > 16) {
		LightType = LightType::None;
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
	MakePoint2();					//
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
		PlaySampleAtObject(Common, BLOKPLOP);

	if (Counter++ > lifeTime || bounces > 16) {
		LightType = LightType::None;
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
	if (poweredUp)
		MakePoint2();
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
		MakePoint1();
		Explode();
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
			MakePoint1();
		else //in early moments, adjust PositionY to follow shooter's
			PositionY = Parent->PositionY - DistanceFromParentY;
	}
}

TNTBullet::TNTBullet(ObjectStartPos& start) : Interactive(start, false) {
	AnimID = 59;
	CollisionShapes.emplace_back(11);
	MakePoint1();
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
			for (const auto& it : HostLevel.Objects) {
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
			PlaySample(Common,EXPL_TNT, sf::Vector2f(PositionX,PositionY), 128);

			AddExplosion(0, 77);
			DoBlast(96*96, true);
				
			MakeBright(18);
		} else {
			MakeBright(--light);
			if (light < 2) 
				Delete();
		}
	}
}
void TNTBullet::Draw(Layer* layers) const {
	if (ObjectType == BunnyObjectType::Interactive)
		DrawNormally(layers);
	else if (Counter < 20) {
		const float scale = (((256 + (Counter-50) * (Counter-50)) / 4) & 255) / 32.f;
		layers[SPRITELAYER - 1].AppendResizedSprite(SpriteMode::Paletted, int(PositionX), int(PositionY), GetFrame(), scale,scale);
	} //else don't draw, let the Explosion object handle that
}

RFBullet::RFBullet(ObjectStartPos& start, bool poweredUp) : PlayerBullet(start, Weapon::RF, 9, true) {
	AnimID = poweredUp ? 51 : 45;
	CollisionShapes.emplace_back(5);
	SpeedX = poweredUp ? 1.5f : 1;
	AccelerationX = 0.1875;
	killAnimID = 3;
	damage = 2;
	lifeTime = poweredUp ? 35 : 40;
	MakePoint2();
}
void RFBullet::Move(GameState& gameState) {
	Event eventAtPixel;
	if (Counter > lifeTime)
		Explode();
	else if (Counter > 0 && gameState.MaskedPixel(int(PositionX), int(PositionY), eventAtPixel) && (eventAtPixel.ID != EventIDs::RICOCHET || !Ricochet())) {
		Explode();
	} else {
		PositionX += SpeedX + pxSpeed;
		PositionY += SpeedY;

		SpeedX += AccelerationX;
		SpeedY += AccelerationY;

		ApproachZeroByUnit(pxSpeed, 0.125f);

		DetermineFrame(gameState.GameTicks >> 2);
		if (++Counter >= CounterMustBeAtLeastThisHighToDrawBullet && !(Counter & 7)) {
			//if (obj->ypos>level.waterlevel)
			//	AddObject(obj->xpos,obj->ypos,aBUBBLE,0);
			//else
				AddExplosion(0, 71);; //smoke particle
		}
	}
}
void RFBullet::Explode() {
	DoBlast(50*50);	

	//if ((obj->load==aPLAYERBULLETP5) && parEnableLighting && !parLowDetail)
		//AddHalo(obj->xpos,obj->ypos);

	PlaySample(Ammo,BOEM1, sf::Vector2f(PositionX,PositionY), 128);

	PlayerBullet::Explode(); //use standard killanim/deleting code
}
RFBullet& RFBullet::AddHorizontalRFBullet(GameObject& parent, sf::Vector2f position, float xSpeed,float ySpeed, bool poweredUp, bool right) { //for keeping SpeedX values constant at various angles, for RF kick purposes
	RFBullet& newRFBullet = static_cast<RFBullet&>(parent.AddObject(poweredUp ? EventIDs::PLAYERBULLETP5 : EventIDs::PLAYERBULLET5, parent.PositionX, parent.PositionY, true));
	if (!right) {
		newRFBullet.SpeedX = -newRFBullet.SpeedX;
		newRFBullet.AccelerationX = -newRFBullet.AccelerationX;
		newRFBullet.DirectionX = -1;
	} else
		newRFBullet.DirectionX = 1;
	newRFBullet.pxSpeed = xSpeed + newRFBullet.SpeedX;
	newRFBullet.SpeedY = ySpeed;
	return newRFBullet;
}

SeekerBullet::SeekerBullet(ObjectStartPos& start, bool poweredUp) : PlayerBullet(start, Weapon::Seeker) {
	AnimID = poweredUp ? 40 : 35; //facing down; each version has five animations for drawing at different angles
	CollisionShapes.emplace_back(4);
	SpeedX = poweredUp ? 1.5f : 2;
	AccelerationX = 0.25f;
	killAnimID = 5;
	damage = 2 + poweredUp;
	lifeTime = 2 * AISPEED;
	MakePoint2();
}
void SeekerBullet::Move(GameState& gameState) {
	if (!Started) {
		Started = true;
		SpeedX += pxSpeed;
		Orphan(); //no longer use the Bunny object for Parent, because that system is about to be repurposed for the missile's target
	}
	
	if (Parent == nullptr) { //no current target; try to find one!
		float mindist = 256*256;
		GameObject* target = nullptr;
		for (auto& it : HostLevel.Objects) {
			if (it->ObjectType == BunnyObjectType::Interactive && static_cast<const Interactive&>(*it.get()).IsEnemy) {
				const auto dx = PositionX - it->PositionX;
				const auto dy = PositionY - it->PositionY;
				const auto distance = dx * dy;
				if (distance < mindist && !TraceLine(gameState, PositionX,PositionY, it->PositionX, it->PositionY)) {
					mindist = distance;
					target = &*it;
				}
			}
		}
		if (target != nullptr)
			target->Adopt(*this);
	}

		
	if (Parent != nullptr)
	{
		float dx = Parent->PositionX - PositionX;
		float dy = Parent->PositionY - PositionY;
		{
			float distance;
			{
				const auto dx2 = dx/4;
				const auto dy2 = dy/4;
				distance=(dx2*dx2+dy2*dy2);
			}

			if (distance<2048)
				distance=sqrt(distance);
			else
				distance=64;	//whatever

			if (distance>1) {
				distance=(3*256)/distance;

				dx=(dx*distance)/128;
				dy=(dy*distance)/128;
			}
		}
		SpeedX = (dx + SpeedX) / 8;
		SpeedY = (dy + SpeedY) / 8;
		DirectionX = (SpeedX < 0) ? -1 : 1;
	}

	PositionX += SpeedX;
	PositionY += SpeedY;

	if (++Counter > lifeTime || CheckFullPixel(gameState, int(PositionX), int(PositionY)))
	{
		Explode();
		return;
	}

	 //chose angle
	AnimID = (damage == 3) ? 40 : 35;
	if (abs(SpeedX) < 0.5f) {
		if (SpeedY < 0.f)
			AnimID += 3; //up
	} else if (abs(SpeedY) < 0.5f)
		AnimID += 2; //horizontal
	else if (SpeedY < 0.f)
		AnimID += 4; //diagonal up
	else
		AnimID += 1; //diagonal down
	DetermineFrame(gameState.GameTicks >> 2);
}
void SeekerBullet::Explode() {
	//cLIGHTEXPLODE();

	PlayerBullet::Explode(); //use standard killanim/deleting code
}

bool SeekerBullet::CheckFullPixel(const GameState& gameState, int x, int y) { //ignores masked tiles depending on certain events
	const EventID eventAtPixel = gameState.GetEvent(x / TILEWIDTH, y / TILEHEIGHT).ID;
	return
		eventAtPixel != EventIDs::ONEWAY &&
		eventAtPixel != EventIDs::VINE &&
		eventAtPixel != EventIDs::HOOK &&
		gameState.MaskedPixel(x,y);
} //CheckFullPixel()
int SeekerBullet::TraceLine(const GameState& gameState, float x0, float y0, float x1, float y1) {
	float dx=(x1-x0)/64;
	float dy=(y1-y0)/64;
	int t=0;

	do {
		++t;

		if (CheckFullPixel(gameState, int(x0),int(y0)))
			return t;

		x0+=dx;
		y0+=dy;
	} while (t<64);

	return 0;
} //TraceLine()
