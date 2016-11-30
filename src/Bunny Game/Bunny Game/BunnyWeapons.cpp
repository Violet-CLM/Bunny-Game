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
					position.y -= 524288;
				speed += (99 - fireSpeed) / 32.f;
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
			pxSpeed = xSpeed;
			if (fireType != Weapon::RF) {
				if (pxSpeed < -8) pxSpeed = -8;
				else if (pxSpeed > 8) pxSpeed = 8;
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

PlayerBullet::PlayerBullet(ObjectStartPos& objStart, Weapon::Weapon id) : BunnyObject(objStart), ammoID(id) {
	ObjectType = BunnyObjectType::PlayerBullet;
}
void PlayerBullet::Behave(GameState& gameState) {
	//todo?
	Move(gameState);
}
void PlayerBullet::Aim(float targetAngle, float xSpeed, float pxSpeedNew, bool reduceLifetime) {
	if (reduceLifetime) {
		lifeTime <<= 1; lifeTime /= 3;
	}
	if (!xSpeed)
		xSpeed = SpeedX;
	pxSpeed = pxSpeed;

	float ySpeed = -cos(targetAngle) * SpeedX;
	xSpeed *= sin(targetAngle);
	
	const float xRatio = (xSpeed / SpeedX);
	const float yRatio = (ySpeed / SpeedX);
	if (AccelerationY)
		AccelerationY *= abs(xRatio);
	AccelerationY += AccelerationX * yRatio;
	if (SpeedY)
		SpeedY *= abs(xRatio);
	SpeedY += SpeedX * yRatio;
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


void BlasterBullet::Move(GameState &) {
	//todo obviously
	PositionX += (SpeedX += AccelerationX) + pxSpeed;
	PositionY += (SpeedY += AccelerationY);
}

/*void BlasterBullet::Draw(Layer* layer) const {
	const auto angle = atan2(SpeedX, SpeedY);
}*/

BlasterBullet::BlasterBullet(ObjectStartPos& objStart, bool poweredUp) : PlayerBullet(objStart, Weapon::Blaster) {
	AnimID = poweredUp ? 20 : 17;
	SpeedX = float(6 + poweredUp);
	AccelerationX = poweredUp ? 0.1875f : 0.125f;
	killAnimID = 4;
	damage = 1 + poweredUp;
	lifeTime = AISPEED / 2 - poweredUp*5;
	//obj->lighttype = 2 - poweredUp;
}
