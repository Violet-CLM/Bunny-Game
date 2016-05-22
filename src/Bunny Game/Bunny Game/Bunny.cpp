#include "Bunny.h"
#include "BunnyObjectList.h"


Bunny::Bunny(ObjectStartPos & objStart) : BunnyObject(objStart), SpeedX(0), platformType(PlatformTypes::None), AccelerationX(0), SpeedY(0), AccelerationY(0), platform_relX(0), platform_relY(0), freeze(0), invincibility(0), airBoard(0), helicopter(0), helicopterTotal(0), specialJump(0), dive(0), lastDive(0), fire(0), lastFire(0), lastDownAttack(0), hit(0), hDir(0), vDir(0), moveSpeedX(0), moveSpeedY(0), fixScrollX(0), quakeX(0), warpCounter(0), frogMorph(0), bossActive(0), vPole(0), swim(0), stop(0), stoned(0), stonedLen(0), spring(0), specialMove(0), slope(0), shiftPositionX(0), runDash(0), run(0), lastRun(0), rolling(0), quake(0), platform(0), ledgeWiggle(0), lastSpring(0), lastJump(0), idleTime(0), hPole(0), hang(0), vine(0), goUp(0), goRight(0), goLeft(0), goDown(0), goFarDown(0), fly(0), fixStartX(0), downAttack(DOWNATTACKLEN), charCurr(0), characterIndex(0), beMoved(0)
{
	AnimID = 67;
	DetermineFrame(1);
	DirectionX = DirectionY = 1;	 
}
//void Bunny::Draw(Layer *) const
//{
//}

void Bunny::GetInput(const KeyStates& keys) {
	//todo base all these codes on JAZZ2.CFG
	KeyDown = keys.Key(sf::Keyboard::Down);
	KeyUp = !KeyDown && keys.Key(sf::Keyboard::Up);
	KeyRight = keys.Key(sf::Keyboard::Right);
	KeyLeft = !KeyRight && keys.Key(sf::Keyboard::Left);
	KeyFire = keys.Key(sf::Keyboard::Space);
	KeySelect = keys.Key(sf::Keyboard::Return);
	KeyRun = keys.Key(sf::Keyboard::LShift) || keys.Key(sf::Keyboard::RShift);
	KeyJump = keys.Key(sf::Keyboard::LControl) || keys.Key(sf::Keyboard::RControl);
}

void Bunny::ProcessInputNoAirboard() {
	if (fly <= 1 /*|| gameObjects[0][fly - 1].behavior == copter*/) { //todo copter
		/*if (swim) { //todo swimming
			downAttack = DOWNATTACKLEN;
			runDash = lastRun = idleTime = 0;
			if (move->hDir) {
				int xSpeed = xSpeed;
				if (move->hDir & 0x8) { //left
					xAcc = -4096;
					if (run) {
						if (xSpeed < -4 * FIXMUL)
							xSpeed = -4 * FIXMUL;
					}
					else if (xSpeed < -2 * FIXMUL)
						xSpeed = -2 * FIXMUL;
				}
				else { //right
					xAcc = 4096;
					if (run) {
						if (xSpeed > 4 * FIXMUL)
							xSpeed = 4 * FIXMUL;
					}
					else if (xSpeed > 2 * FIXMUL)
						xSpeed = 2 * FIXMUL;
				}
			}
			else {
				xAcc = 0;
				xSpeed = 31 * xSpeed / 32;
			}
		}
		else*/ if (!(helicopter /*&& characterProfile[characterIndex].helicopterXSpeed*/)) { //not swimming, not controlled by helicopter xSpeed //todo? character profiles
			if (runDash >= 0 && !beMoved) {
				if (KeyLeft || KeyRight) {
					runDash = 0;
					if (hit) {
						//do nothing
					} else if (run) {
						if (KeyRight)
								AccelerationX = 0.366210938f;
						else
							AccelerationX = -0.366210938f;
					} else if (KeyRight)
							AccelerationX = 0.183105469f;
					else
						AccelerationX = -0.183105469f;
				}
				else { //not pressing left or right
					AccelerationX = 0;
					if (hang)
						SpeedX = 0;
					else {
						if (SpeedX) {
							const float maxSpeedX = run ? 0.427246094f : 0.122070312f;
							if (SpeedX > 0) {
								if (SpeedX > maxSpeedX)
									SpeedX -= maxSpeedX;
								else
									SpeedX = 0;
							} else {
								if (SpeedX < -maxSpeedX)
									SpeedX += maxSpeedX;
								else
									SpeedX = 0;
							}
						}
					}
				}
			} //else don't change speed
		}
	}
	/*else { //copter or cheshire2 //todo cheshire cats
		TgameObj* flyingObject = &gameObjects[0][fly - 1];
		jjBEHAVIOR& behavior = flyingObject->behavior;
		if (behavior == loadObjects[aCHESHIRE2].behavior) {
			if (goUp) {
				xSpeed = flyingObject->xPos - xPos;
				int newYSpeed;
				if (getPlayerVarSettingLocal(play, pvANTIGRAV)) { //it might be better to do the job inside the cheshire2 code instead of here, but this is easier
					flyingObject->yPos -= flyingObject->ySpeed * 2;
					flyingObject->direction = 0x40;
					newYSpeed = (40 * FIXMUL) - flyingObject->yPos + yPos;
				}
				else {
					flyingObject->direction = 0;
					newYSpeed = (40 * FIXMUL) - yPos + flyingObject->yPos;
				}
				ySpeed = newYSpeed;
				xAcc = 0;
				yAcc = 0;
				ySpeed = newYSpeed - ((newYSpeed <= 0) ? 8192 : 24576);
			}
			else {
				flyingObject->state = sDONE; //I think this is right
				flyingObject->direction = 0;
				fly = 0;
			}
		}
	}*/
}
void Bunny::ProcessInputJumpFallStuff() {
	if (!KeyJump || beMoved) { //not jumping: accelerate downwards
		if (spring == 0 && stop == 0 && swim == 0) {
			if (SpeedY >= -1) {
				if (SpeedY >= -0.25) {
					if (SpeedY < 0)
						SpeedY = 0;
				}
				else
					SpeedY += 0.25;
			}
			else
				SpeedY += 0.5;
		}
		++lastJump;
		return;
	}
	//for everything after this, move->jump == 1
	if (swim) {
		/*if (PositionY < GameGlobals->level.waterLevel + 32 * FIXMUL) { //todo swimming
			PlaySample(PositionX, PositionY, (characterIndex != char2LORI) ? sCOMMON_JUMP : (RandFac(3) + sLORISOUNDS_LORIJUMP), 40, 0);
			swim = spring = helicopterTotal = specialJump = 0;
			SpeedY = jumpSpeed - 4 * FIXMUL;
		}*/
		return;
	}
	if (fly > 1) {
		lastJump = 0;
		return;
	}
	if (SpeedY > 0 || (SpeedY < 0 && helicopter)) { //helicopter check is new
		if (lastJump >= 5 && lastJump < 30) { //press jump in midair
			/* //todo double jumping stuff
			tCharacterProfile* profile = &characterProfile[characterIndex];
			if (profile->airJump == airjumpSPAZ) { //change, -ish
				if (specialJump < profile->doubleJumpCountMax) {
					++specialJump;
					if (onWallClimb()) //this part can (and nearly must) be done even if you're buttstomping
						PositionY -= 4 * FIXMUL;
					if (downAttack >= DOWNATTACKLEN) { //changed
						SpeedX = (direction >= 0) ? profile->doubleJumpSpeedX : -profile->doubleJumpSpeedX;
						SpeedY = profile->doubleJumpSpeedY;
						spring = 0;
						PlaySample(PositionX, PositionY, sCOMMON_UP, 0, 0);
					}
				}
			}
			else if (profile->airJump == airjumpHELICOPTER) { //Jazz or Lori
				if (helicopterTotal < profile->helicopterDurationMax) {
					helicopter = AISPEED;
					lastJump = 15; //was: 30
				}
			}
			*/
			return;
		}
	}
	if (fly != 0)
		return;
	if (goDown == 0) {
		helicopterTotal = specialJump = 0;
		/*if (lift >= PLAYSLOT) { //todo cooperative lifting
			Tplayer* liftPlay = &player[lift - PLAYSLOT];
			liftSpeedY = -1048576;
			liftPositionY -= 262144;
			liftspring = playerID + PLAYSLOT;
			liftlastSpring = *gameTicks;
			liftidleTime = 0;
			liftfixAnim = 0;
			lift = 40;
			lastJump = 0;
			return;
		}*/
		if (dive > 4 && SpeedX == 0) { //dive means "crouch", I think?
			if (specialMove == 0) {
				specialMove = 1;
				spring = 0;
				lastJump = 0;
				return;
			}
		}
		else if (platform == 0 || platformType != 4) {
			//PlaySample(PositionX, PositionY, (characterIndex != char2LORI) ? sCOMMON_JUMP : (RandFac(3) + sLORISOUNDS_LORIJUMP), 40, 0); //todo sample
			SpeedY = -10 - (abs(SpeedX) / 4); //jumpSpeed = -10
		}
		spring = 0;
		lastJump = 0;
		return;
	}
	else {
		if (!(((hang == 2) && (SpeedY != 0 || lastJump <= 2)) || hang == 0 || SpeedY != 0)) { //was <= 8, but that was more sensitive than needed just for preventing jumping up through hooks
			//PlaySample(PositionX, PositionY, (characterIndex != char2LORI) ? sCOMMON_JUMP : (RandFac(3) + sLORISOUNDS_LORIJUMP), 40, 0); //todo sample
			SpeedY = -10;
			helicopterTotal = specialJump = 0;
			//if (!getPlayerVarSettingLocal(play, pvANTIGRAV)) //todo no fire zones
				PositionY -= 4;
			//else
			//	PositionY += 4;
			spring = 0;
		}
	}
	lastJump = 0;
}
void Bunny::ProcessInput()
{
	if (freeze) {
		if (KeyFire) {
			if (!fire) {
				if (freeze <= 8) {
					freeze = 0;
					//cUNFREEZE(playerID + PLAYSLOT, 0); //todo cUNFREEZE
				}
				else
					freeze -= 8;
			}
		}
		else
			fire = 0;
		AccelerationX = 0;
		SpeedX = 31 * SpeedX / 32;
		return;
	}
#ifdef LEDGE_CLIMBING
	if (climbLedge)
		return;
#endif
	{ //can the player move at all?
		bool immobilized = (hit || warpCounter > 0);
		immobilized |= (--hPole > -10);
		immobilized |= (--vPole > -10);
		/*if (finished || GameGlobals->level.finish || extraLives < 0) { //menu
			immobilized = true;
			invincibility = -1;
		}*/
		//if ((MenuGlobals->menuCode & quitWAIT) && (move->fire || move->jump)) //todo menu
			//MenuGlobals->menuCode &= quitAND;
		//if (move->fire && getPlayerVarSettingLocal(play, pvNOFIRE)) //todo no fire zones
			//move->fire = 0;
		/*if (GameGlobals->gameMode == GM_RACE && !IsLocalGameOrServer() && serverPlusVersion < PLUSVER_5_0) { //this countdown is in entirely the wrong spot, if you ask me //todo? race mode
			int currentGameTick = *gameTicks;
			if (currentGameTick == 140 || currentGameTick == 210 || currentGameTick == 280)
				PlaySample(0, 0, sCOMMON_BELL_FIRE2, 0, 0);
			if (currentGameTick < 350)
				immobilized = true; //though I guess this part makes some sense
			else if (currentGameTick == 350) {
				lastLapFinish = 350;
				PlaySample(0, 0, sCOMMON_BELL_FIRE, 0, 0);
			}
		}*/
		immobilized |= (specialMove > 10);
		if (immobilized) {
			//*(short *)(move) &= 0xE500; //todo!
			lastFire = 800;
		}
		/*else if (fly > 1 && gameObjects[0][fly - 1].load == aCHESHIRE2)//todo cheshire cats
			*(BYTE *)move = 0;*/ 
}
	if (stonedLen > 0) {
		if (KeyLeft) {
			KeyRight = true; KeyLeft = false;
		}
		else if (KeyRight) {
			KeyLeft = true; KeyRight = false;
		}
		if (KeyUp) {
			KeyDown = true; KeyUp = false;
		}
		else if (KeyDown) {
			KeyUp = true; KeyDown = false;
		}
		KeyRun = false;
		if (++stoned == stonedLen) {
			stoned = 0;
			stonedLen = 0;
		}
		idleTime = 0;
	}
	/*if (int lift = lift) { //todo? cooperative lifting
		if (lift >= PLAYSLOT)
			*(BYTE *)move = 0;
		else if (lift < 70) {
			if ((lift = --lift) == 32 || !move->jump) {
				SpeedY = -6 * FIXMUL;
				PositionY -= FIXMUL;
				spring = lift;
				lastSpring = *gameTicks;
			}
			if (move->jump)
				move->jump = 0;
			else
				lift = 0;
		}
	}*/
	/*if (int pushObject = pushObject) { //todo pushing
		jjBEHAVIOR& behavior = gameObjects[0][pushObject].behavior;
		if (behavior == boxx || behavior == cBIGOBJECT) //todo again
			KeyFire = 0;
	}*/
	if (bossActive < 0) //I wonder if this ever gets used?
		invincibility = -AISPEED;
	//if (charCurr == mBIRD)
		//return ProcessBirdInput(play, move);
	if (frogMorph)
		--frogMorph;
	if (beMoved > 0)
		--beMoved;
	//if (characterIndex == char2FROG)
		//return ProcessFrogInput(play, move);
	if (hang == 2) { //hook, not vine
		AccelerationX = 0;
		SpeedX = 0;
		if (KeyLeft)
			DirectionX = -1;
		else if (KeyRight)
			DirectionX = 1;
		KeyLeft = KeyRight = false;
	}
	if (fly != -1 || beMoved || vPole >= 0 || hPole >= 0) {
		ProcessInputNoAirboard();
	}
	else { //airboard
		/*loopSample = PlayLoopSample(PositionX, PositionY, sCOMMON_AIRBOARD, //todo sounds
			min(48, (abs(SpeedX) >> 16) + 8),
			(goDown) ? min(26000, (goFarDown << 6) + 10.122070312) : 10.122070312,
			loopSample
		);*/
		/*if (airBoard == 2) //todo sounds
			PlaySample(PositionX, PositionY, sCOMMON_AIRBTURN2, 100, 16537);
		else if (airBoard == 12)
			PlaySample(PositionX, PositionY, sCOMMON_AIRBTURN, min(48, abs(SpeedX) / 65536 + 32), 16537);*/
		if (!(hDir)) {
			AccelerationX = 0;
			SpeedX = 31 * SpeedX / 32;
			if (airBoard < 14) {
				if (--airBoard <= 0) {
					airBoard = 14;
					DirectionX = -DirectionX;
				}
			}
		}
		else if (hDir & 0x8) { //left
			AccelerationX = -0.125f;
			if (airBoard < 14) {
				if (--airBoard <= 0) {
					airBoard = 14;
					DirectionX = -1;
				}
			}
			if (DirectionX >= 0 && airBoard == 14)
				airBoard = 13;
		}
		else { //right
			AccelerationX = 0.125f;
			if (airBoard < 14) {
				if (--airBoard <= 0) {
					airBoard = 14;
					DirectionX = 1;
				}
			}
			if (DirectionX < 0 && airBoard == 14)
				airBoard = 13;
		}
	} //had airboard
	if (!airBoard) {
		if (KeyLeft)
			DirectionX = -1;
		else if (KeyRight)
			DirectionX = 1;
	}
	++lastDownAttack;
	if (hit || /*GameGlobals->level.finish ||*/ warpCounter > 0) { //todo cycling
		downAttack = DOWNATTACKLEN;
		++lastRun;
		++lastFire;
		if (fly > 1) {
			/*TgameObj* flyingObject = &gameObjects[0][fly - 1]; //todo copters
			if (flyingObject->behavior == copter)
				flyingObject->state = sDONE; //again, I _think_ this is what that code does*/
			fly = 0;
		}
		fire = 0;
		spring = 0;
		runDash = 0;
		idleTime = 0;
		hDir = 0;
		vDir = 0;
		if (run > 0)
			if ((run -= 4) < 0)
				run = 0;
	}
	else {
		{
			if (hang || vine) {
				if (!KeyDown && goDown)
					downAttack = (DOWNATTACKLEN + AISPEED);
				else
					downAttack = DOWNATTACKLEN; //to prevent the player from buttstomping next tick if keyDown is still pressed
			}
			else if (goDown) {
				if (KeyDown) {
					if (downAttack >= (DOWNATTACKLEN + AISPEED))
						helicopter = downAttack = 0; //start a buttstomp
				}
				else if (downAttack >= DOWNATTACKLEN)
					downAttack = (DOWNATTACKLEN + AISPEED);
			}
			if (downAttack <= DOWNATTACKWAIT || downAttack >= DOWNATTACKLEN)
				++downAttack;
			if (downAttack < DOWNATTACKLEN)
				lastDownAttack = dive = 0;
			downAttack = downAttack;
		}
		if (KeyRun){// && characterProfile[characterIndex].canRun) { //changed //todo? character profiles
			run = std::min(32, run + 1);
			if (lastRun > 2 && lastRun < 15 && !runDash)
				runDash = 1;
			lastRun = 0;
		}
		else {
			run = std::max(0, run - 2);
			++lastRun;
		}
		if (!SpeedX && !AccelerationX && !SpeedY && !hang) { //start up a runDash
			if (runDash <= 0 || lastRun <= 0 || lastRun >= 15) {
				if (lastRun > 15) {
					if (runDash <= 20)
						runDash = runDash = 0;
					else {
						//PlaySample(PositionX, PositionY, sAMMO_BULFL2, 0, 0); //todo sounds
						if (runDash > 80)
							runDash = runDash = 80;
						SpeedX = (DirectionX * runDash) / 4.f;
						AccelerationX = SpeedX / 32.f;
						runDash = 4 * (-250 - runDash);
					}
				}
			}
			else if ((runDash = ++runDash) & 3)
				{}// PlaySample(PositionX, PositionY, sCOMMON_REVUP, 20, 0); //todo sounds
		}
		/*if (vine && (move->jump || move->fire)) { //todo swinging vine
			TgameObj* swingingVine = &gameObjects[0][vine];
			swingingVine->state = sSLEEP;
			swingingVine->counter = 35;
			if (move->jump) {
				SpeedY -= 524288;
				move->jump = 0;
			}
			if (move->fire) {
				SpeedY -= 65536;
				move->fire = 0;
			}
			vine = 0;
		}
		else*/
			//ProcessSelectInput(play, move); //todo!

		ProcessInputJumpFallStuff();
		//ProcessRabbitInputStuffWithFlyAndSwim(play, move); //todo!
	}
}

#include "Windows.h"
#include "Misc.h"
void Bunny::DoLandscapeCollision(GameState& gameState)
{
	int px, py;
	int ty;
	int check, calc;
	//	int hurt;
	int firstPixelY;
	int checkCeiling;
	int backtrace;
	float mSpeedX, mSpeedY;
	float newPositionX, newPositionY;
	int stepN;
	float stepX, stepY;
	int checkY;
	Event tileAttr;

	/*	this function checks the player against a 24x24 pixel block, which
	should be completely empty for the player to walk through.

	totally new method:

	goLeft,goRight aren't necessarily always updated,
	they only scan in the direction that you're facing+going in.
	that's pretty much like nature behaves, doesn't matter too much

	goUp/goDown ARE updated (needed for jumping)
	*/
	int bx = int(PositionX / TILEWIDTH);
	int by = int(PositionY / TILEHEIGHT);

	const float playerGravity = 0.125f;
	const int gravDir = (DirectionY < 0) ? -1 : 1; // oodles of numbers will now get multiplied by gravDir.
	//noGun &= 0xFFFF; //not in a local-toggling no fire zone //todo?

	/*if (GameGlobals->level.waterLevel < 0xfffffff) {
		if (PositionY > 12 + GameGlobals->level.waterLevel) {
			if (!swim) {
				AddExplosion(PositionX, GameGlobals->level.waterLevel, AnimBase[mCOMMON] + mCOMMON_SPLASH);
				PlaySample(PositionX, GameGlobals->level.waterLevel, sCOMMON_WATER, 0, 0);
				helicopter = 0; //violet. coptering into water made it impossible to move up from ground
									  // also, isn't there a bug where you can't fly up off a monitor if
									  // you have an airboard? that should get fixed!
			}
			swim = 1;
			fly = 0;	//to be sure
			if ((*gameTicks & 63) < 31 && RandFac(7) == 0) {
				AddObject(PositionX + direction * 16, PositionY - 8, aBUBBLE, 0);
			}
		}
		else if (PositionY < 4 + GameGlobals->level.waterLevel) {
			swim = 0;
		}
	}
	else {	//to be sure (waterLevel goes away)
		swim = 0;
	}*/ //todo swimming

	if (platformType == PlatformTypes::SuckerTube) { // violet's noclip mode
		if (warpCounter <= 0) { // probably other checks would be good here too, but that's the only one I thought of
			PositionX += (SpeedX = platform_relX);
			PositionY += (SpeedY = platform_relY);
		}
		rolling = beMoved = 10; //largely arbitrary number that keeps getting reset until the state's over
		hang = 0;
		goLeft = true;
		goRight = true;
		goDown = true;
		goUp = true;

		return;
	}

	// back to non-violet stuff
	if (vPole > 0 || hPole > 0) {
		SpeedX = 0;
		AccelerationX = 0;
		SpeedY = 0;
		AccelerationY = 0;

		return;
	}

	//X SPEED
	SpeedX += AccelerationX;

	if (SpeedX < 0 && !goLeft) {
		SpeedX = 0;
		AccelerationX = 0;
	} else if (SpeedX > 0 && !goRight) {
		SpeedX = 0;
		AccelerationX = 0;
	}

	if (true) {// if (IsRabbit(characterIndex)) {	//todo !BIRD  and !FROG!!!!!!!!
		if (stonedLen) {
			if (SpeedX > 2) {
				SpeedX = 2;
			}
			else if (SpeedX < -2) {
				SpeedX = -2;
			}
			mSpeedX = SpeedX + moveSpeedX;
		}
		else if (run || runDash < 0 || beMoved || specialMove) {
			if (!beMoved) {
				if (SpeedX > 16) {
					SpeedX = 16;
				}
				else if (SpeedX < -16) {
					SpeedX = -16;
				}
			}
			mSpeedX = SpeedX + moveSpeedX;
			if (!beMoved) {
				if (mSpeedX > 8) {
					mSpeedX = 8;
				}
				else if (mSpeedX < -8) {
					mSpeedX = -8;
				}
			}
		}
		else {
			if (SpeedX > 4) {
				SpeedX = 4;
			}
			else if (SpeedX < -4) {
				SpeedX = -4;
			}
			mSpeedX = SpeedX + moveSpeedX;
		}

		if (downAttack < DOWNATTACKLEN) {
			SpeedX = 0;
			if (downAttack >= DOWNATTACKWAIT) {
				if (downAttack == DOWNATTACKWAIT) {
					//if (characterIndex == char2SPAZ) { //todo sound
					//	PlaySample(PositionX, PositionY, sSPAZSOUNDS_YAHOO, 0, 0);
					//}
				}
				SpeedY = 10;
				spring = 0;
			}
			else {
				SpeedY = 0.0625f;
			}
		} else if (platform == 0 || platformType == 0) {
			if (helicopter > 0) { //violet: adding the !fly check gets rid of the old coptering-onto-a-fly-powerup bugs
				if (goDown) {
					SpeedY += AccelerationY*playerGravity / 1.25f;
				}
			}
			else if (!hang) {
				if (goDown && !stop) {
					/*if (PositionY > GameGlobals->level.waterLevel) { //todo water
						//SpeedY += AccelerationY+1024;	//+(playerGravity/1.25f)-1;
						SpeedY += AccelerationY*playerGravity / 1.25f + 1024;
					}
					else */{
						if (SpeedY < 0) {
							SpeedY += AccelerationY + 3 * playerGravity;
						}
						else {
							SpeedY += AccelerationY + playerGravity;
						}
					}
				}
				else if (goDown || AccelerationY < 0) {
					//SpeedY += AccelerationY;	//+(playerGravity/1.25f)-1;
					SpeedY += AccelerationY*playerGravity / 0.125f;
				}

				if (lastSpring + 35 < int(gameState.GameTicks) && SpeedY > 0.5f) {
					spring = 0;
				}
			}
		}

		if (SpeedY < -32) {
			SpeedY = -32;
		} else if (SpeedY > 12) {
			SpeedY = 12;
		}

		mSpeedY = SpeedY + moveSpeedY;
		if (mSpeedY > 12) {
			mSpeedY = 12;
		} else if (mSpeedY < -8) {
			mSpeedY = -8;
		}
	}
	/*else { //bird or frog
		if (run) {
			if (SpeedX > 8) {
				SpeedX = 8;
			}
			else if (SpeedX < -8) {
				SpeedX = -8;
			}
		}
		else {
			if (SpeedX > 4) {
				SpeedX = 4;
			}
			else if (SpeedX < -4) {
				SpeedX = -4;
			}
		}
		mSpeedX = SpeedX;

		if (charCurr == mBIRD) {
			if (SpeedY > 4) {
				SpeedY = 4;
			}
			else if (SpeedY < -4) {
				SpeedY = -4;
			}
			mSpeedY = SpeedY;

		}
		else if (characterIndex == char2FROG) {

			if (goDown && !stop) {
				if (SpeedY < 0) {
					SpeedY += AccelerationY + 3 * playerGravity;	//3*1.25f;	//otherwise jumps to high
				}
				else {
					SpeedY += AccelerationY + playerGravity;	//1.25f;//GameGlobals->level.gravity;
				}
			}
			else if (goDown || AccelerationY < 0) {
				//SpeedY += AccelerationY;	//+(playerGravity/1.25f)-1;
				SpeedY += AccelerationY*playerGravity / 1.25f;
			}

			if (SpeedY < -32) {
				SpeedY = -32;
			}
			else if (SpeedY > 12) {
				SpeedY = 12;
			}
			mSpeedY = SpeedY;

			if (mSpeedY > 12) {
				mSpeedY = 12;
			}
			else if (mSpeedY < -8) {
				mSpeedY = -8;
			}
		}
	}*/


	//check for conveyor belts

	if (!goDown) {
		bx = int(PositionX + 15) / TILEWIDTH;
		by = int(PositionY + (24 * gravDir)) / TILEHEIGHT;
		Event floorEvent = gameState.GetEvent(bx, by);
		bx = calc = floorEvent.GetParameter(0, -8);
		if (!calc) calc = 2;

		switch (floorEvent.ID) {
		case EventIDs::BELTRIGHT:
			mSpeedX += calc;
			break;
		case EventIDs::BELTLEFT:
			mSpeedX -= calc;
			break;
		case EventIDs::BELTACCRIGHT:
			SpeedX += calc;
			mSpeedX += calc;
			break;
		case EventIDs::BELTACCLEFT:
			SpeedX -= calc;
			mSpeedX -= calc;
			break;
			// violet
			// ideally this would affect the deceleration somehow, instead of continually
			// upping the speed, but doing it this way is a lot easier.
		case EventIDs::SLIDE:
			bx &= 3;
			if (SpeedX && (
				!AccelerationX ||
				(AccelerationX < 0 && SpeedX > 0) ||
				(AccelerationX > 0 && SpeedX < 0)
				)) {
				SpeedX += (((run) ? (16000 + bx * 2500) : (4000 + bx * 800)) * ((SpeedX > 0) ? 1 : -1)) / 65536.f;
			}
			break;
		}
	}
	//check for wind
	//need to build in code that finds these events, and then
	//executes these parts of code when the events are in a level
	//at all! should speed up a tad
	bx = int(PositionX / TILEWIDTH);
	by = int(PositionY / TILEHEIGHT);
	if (hPole < -5 && vPole < -5) for (int stepY = 0; stepY < 2; stepY++) { //todo wind
		for (int stepX = 0; stepX < 2; stepX++) {
			Event eventHere = gameState.GetEvent(bx + stepX, by + stepY);
			if (eventHere.ID == EventIDs::WINDLEFT) {
				calc = eventHere.GetParameter(0, -8);
				if (!calc) {
					mSpeedX -= 1;
				}
				else {
					mSpeedX += calc * 0.25f;
				}
				idleTime = 0;
			} else if (eventHere.ID == EventIDs::WINDRIGHT) {
				calc = eventHere.GetParameter(0, -8);
				if (!calc) {
					mSpeedX += 1;
				}
				else {
					mSpeedX += calc * 0.25f;
				}
				idleTime = 0;
			} else if (eventHere.ID == EventIDs::FLOATUP) {
				SpeedY -= (8 - abs(SpeedY));
				/*if (characterIndex != char2FROG)*/ AccelerationY = -0.5f; //todo frog?
				mSpeedY = SpeedY;
				if (mSpeedY > 4) {
					mSpeedY = 4;
				}
				idleTime = 0;
			}
		}
	}
	//end check for wind


	/*if (platformType == PlatformTypes::SwingingPlatform) { //todo platforms
		platform_relX += mSpeedX;
		newPositionX = gameObjects[0][platform].PositionX + int(platform_relX);

		if (SpeedY > -1)
			newPositionY = gameObjects[0][platform].PositionY + int(platform_relY);
		else newPositionY = PositionY;
	}
	else */{
		newPositionX = PositionX + mSpeedX;
		newPositionY = PositionY;
	}
	newPositionY += mSpeedY*gravDir;

	//CheckDestructScenery(playerID, newPositionX, newPositionY); //todo destruct scenery

	/*if (*numPlayers > 1 || NetGlobals->partyMode != gameLOCAL) {
		SolidPlayers(play, &newPositionX, &newPositionY);
	}*/	//Multiplayers : solid

	if (shiftPositionX) {
		newPositionX = (newPositionX / 8) * 7 + (shiftPositionX / 8);
		shiftPositionX = 0;
	}

	// jjnowall.
	/*if (*noWallCollision == 1 && fly && GameGlobals->gameMode == GM_SP) { //todo cheat codes
		PositionX = newPositionX;
		PositionY = newPositionY;
		hang = 0;
		goLeft = true;
		goRight = true;
		goDown = true;
		goUp = true;

		return;
	}*/

	float oldSpeedX = SpeedX;
	float oldSpeedY = SpeedY;
	hang = 0;
	slope = 0;


	//LIMIT the new positions!
	// now with Violet's crazy Limit X Scroll parameters.
	/*if (fixStartX) { //todo limit scroll
		px = (fixStartX & 1023) << 21; //left, stored in the first ten bits
		py = ((fixStartX >> 10) & 1023) << 21; //width, stored in the next ten bits
		if (py) {
			PositionX = min(max(PositionX, fixScrollX), fixScrollX + py);
			newPositionX = min(max(newPositionX, fixScrollX), fixScrollX + py);
		}
		else if (newPositionX < px) {
			newPositionX = px;
			if (PositionX < px) PositionX = px;
		}
	}
	else */if (newPositionX < 0) newPositionX = 0;
	/*if (newPositionX > GameGlobals->level.fixWidth) { //todo level bounds
		newPositionX = GameGlobals->level.fixWidth;
	}*/

	if (newPositionY < 0) {
		newPositionY = 0;
	}
	/*else if (newPositionY > GameGlobals->level.fixHeight) { //todo level bounds
		newPositionY = GameGlobals->level.fixHeight;
	}*/

	//set top left corner of checking box (actual mask collision detection begins here!)
	px = int(newPositionX) - 12;
	py = int(newPositionY) - ((gravDir == 1) ? 4 : -2);

	//first: check if we gonna hit a vine
	//if (mSpeedY>=0)
	if (!fly) {// && characterIndex != char2FROG) { //todo frog
		for (ty = (gravDir == 1 ? -8 : 11); ty; ty += gravDir) {
			check = gameState.MaskedHLine(px, py + ty, 24);
			if (!check) {	//really nothing there
				check = gameState.MaskedHLine(px, py + ty, 24, tileAttr);
				if (check && (tileAttr.ID == EventIDs::VINE || tileAttr.ID == EventIDs::HOOK)) {
					if (!hang) {
						if (tileAttr.ID == EventIDs::VINE) {
							hang = 1;
						}
						else {
							hang = 2;	//HOOK
											//newPositionX=((px/32)*32+15);

							SpeedX = 0;
							SpeedY = 0;
						}

						if (SpeedY > 4) {
							//PlaySample(PositionX, PositionY, sCOMMON_FOEW3, 0, 30000); //todo sample
						}
					}

					newPositionY = float(gravDir * 9 + py + ty);

					mSpeedY = newPositionY - PositionY;
					SpeedY = 0;
					AccelerationY = 0;
					break;
				}
			}
		}	//for ty
	} //else helicopter = 0;	//if not a bird/flier: falling

	  //start checking 24x24 block
	firstPixelY = 0;

	for (ty = 1; ty < 24; ++ty) {
		check = gameState.MaskedHLine(px, py + ty*gravDir, 24, tileAttr);
		if (check) {
			if (tileAttr.ID == EventIDs::VINE || tileAttr.ID == EventIDs::HOOK) {
				//do nothing
			}
			else if (mSpeedY < 0 && tileAttr.ID == EventIDs::ONEWAY) {
				//do nothing
			}
			else {
				firstPixelY = ty;
				break;	//break out of ty loop
			}
		}
	}	//for ty

	backtrace = 0;
	if (firstPixelY) {	//if there are any pixels inside this block!
		calc = 20 - int(abs(SpeedX));

		// determine the "backtrace" variable:
		if (firstPixelY >= calc) {	//4=max stephite
			checkCeiling = 0;
			for (ty = 1; ty >= firstPixelY - 24; --ty) {
				check = gameState.MaskedHLine(px, py + ty*gravDir, 24, tileAttr);
				if (mSpeedY < 0 && (tileAttr.ID == EventIDs::ONEWAY || tileAttr.ID == EventIDs::VINE || tileAttr.ID == EventIDs::HOOK)) {
					check = 0;	//override for ONEWAY+VINE event (not bouncing ur head)
				}

				if (check) {
					checkCeiling = ty;
					break;	//break out of ty loop
				}
			}	//for ty //search ceiling

			if (!checkCeiling) {	//safest!
				PositionX = newPositionX;
				if (gravDir == -1) py += 2;
				PositionY = (py + (4 + firstPixelY - 24)*gravDir) + float(gravDir) / 2;
				slope = -1;

				if (SpeedY > 0)
					SpeedY = 0;
				if (AccelerationY > 0)
					AccelerationY = 0;
				backtrace = 0;

			}
			else {
				backtrace = 1;
			}
		}
		else {//if this little ledge is below max stephite
			backtrace = 1;	//do backtrace
		}

		if (backtrace) {
			//start backtracing, to see how far we could still walk in that
			//particular direction

			stepX = (newPositionX - PositionX) / 16;    //8=arbitrary whatever works
			stepY = (newPositionY - PositionY) / 16;	//do not use mSpeedX (clipping!)

			stepN = 0;
			do {
				px = int(PositionX) - 12;
				py = int(PositionY) - (gravDir << 2);

				for (ty = 0; ty < 24; ++ty) {	//check 24x24
					check = gameState.MaskedHLine(px, py + ty*gravDir, 24, tileAttr);
					if (stepY < 0 && (tileAttr.ID == EventIDs::ONEWAY || tileAttr.ID == EventIDs::VINE))

					if (tileAttr.ID == EventIDs::VINE || tileAttr.ID == EventIDs::HOOK)
						check = 0;	//override for ONEWAY+VINE event
					else {
						if (tileAttr.ID == EventIDs::ONEWAY && (mSpeedY < 0 || ty < 20))
							check = 0;
						else if (check){
							break;
						}
					}
				} //for ty

				if (!check) {
					//if everything OK increase positions
					PositionX += stepX;
					PositionY += stepY;
					stepN++;
				}
			} while (!check && stepN < 16);

			{	//if stepped at all!
				PositionX -= stepX;
				PositionY -= stepY;
			}
		}
	}
	else {    //24x24 block is totally empty
		PositionX = newPositionX;
		PositionY = newPositionY;

		//TEMP!!!!!!!!!
		if (!fly && abs(SpeedX) >= 1 && SpeedY > 0) {
			px = int(newPositionX) - 12;
			py = int(newPositionY) - (gravDir << 2);

			checkY = std::min(28 + int(abs(SpeedX)), 32);

			for (ty = 24; ty < checkY; ++ty) {
				if (gameState.MaskedHLine(px, py + ty*gravDir, 24)) {
					PositionY = (newPositionY + (ty - 24)*gravDir) + float(gravDir) / 2;
					slope = 1;
					break;
				}
			}
		}	//fly=0

		if (mSpeedX < 0) {
			goLeft = true;
		}
		else if (mSpeedX > 0) {
			goRight = true;
		}
	}

	px = int(PositionX) - 12;
	py = int(PositionY) - (gravDir << 2);

	goLeft = !gameState.MaskedVLine(px - 1, py, 20 * gravDir);
	goRight = !gameState.MaskedVLine(px + 25, py, 20 * gravDir);
	if (slope == 0) {
		if ((!goRight && SpeedX > 0) || (!goLeft && SpeedX < 0)) {
			SpeedX = 0;
			AccelerationX = 0;
		}
	}

	goUp = !gameState.MaskedHLine(px, py - (gravDir << 3), 24, tileAttr);

	if (tileAttr.ID == EventIDs::VINE || tileAttr.ID == EventIDs::HOOK || (SpeedY < 0 && tileAttr.ID == EventIDs::ONEWAY)) {
		goUp = true;
	}

	if (slope == 0 && !goUp && SpeedY < 0) {
		SpeedY = 0;
	}

	goDown = !gameState.MaskedHLine(px, py + ((gravDir == 1) ? 25 : -25), 24, tileAttr);
	if ((tileAttr.ID == EventIDs::VINE || tileAttr.ID == EventIDs::HOOK) && !hang/* && characterIndex != char2FROG*/) {
		goDown = true;
	}

	 //todo buttstomp
#ifndef NOWALLJUMPING
	if (backtrace && downAttack < DOWNATTACKLEN) {
		goUp = true;
		SpeedY = -4;
		goDown = false;
		lastJump += 15; //so that you don't immediately resume copter-earing
		downAttack = DOWNATTACKLEN;	//crash.... if oneway or backtraced
	}
#endif

	if (slope == 0 && !goDown && SpeedY > 0) {
		SpeedY = 0;
	}

	if (!goDown) {
		goFarDown = false;

		if (downAttack < DOWNATTACKLEN) {
			downAttack = DOWNATTACKLEN;
		}
		else if (SpeedX == 0) {
			px = int(PositionX);

			check = gameState.MaskedVLine(px + (DirectionX * 12), py + (gravDir << 5), 64 * gravDir);
			if (!check) {
				if (!ledgeWiggle) {
					ledgeWiggle = 1;
				}
			}
			else {
				ledgeWiggle = 0;
			}
		}
	}
	else {
		goFarDown = !gameState.MaskedVLine(int(PositionX), py + (gravDir << 5), 64 * gravDir);
	}

	if (abs(oldSpeedX - SpeedX) > 4) {
		quakeX = abs(oldSpeedX - SpeedX) / 1.25f;
	}

	if (!goDown && !fly && abs(oldSpeedY - SpeedY) > 3) {
		//PlaySample(PositionX, PositionY, sCOMMON_LANDPOP, 0, 0); //todo sample

		if (downAttack == DOWNATTACKLEN) {
			//			AddExplosion(PositionX, PositionY+20, AnimBase[mAMMO]+mAMMO_BOOM1);
			//			quakeY = 5+abs(oldSpeedY - SpeedY);
			//			quake = -14;//-quakeY;
			quake = -AISPEED / 4;
		}
		/*else if (abs(oldSpeedY - SpeedY) > 5) { //todo explosions
			AddExplosion(PositionX, PositionY + 16, AnimBase[mAMMO] + mAMMO_POOF2);
		}*/
	}

	/*if ((platform && (platformType != PlatformTypes::None))	//not falling when standing on platform type //todo platforms
		||
		(PositionY == GameGlobals->level.fixHeight && g_hasPits == 2)) {
		//CommandEcho(sprintf_z("Platform: %d. Type: %d", platform, platformType));
		if (goDown && SpeedY > 0) {
			SpeedY = 0;
		}
		goDown = false;
		goFarDown = false;
		if (downAttack < DOWNATTACKLEN)
			downAttack = DOWNATTACKLEN;
	}*/

#ifdef LEDGE_CLIMBING
	if (goFarDown) //don't cling to really short ledges
		ledge = CheckLedge(PositionX, PositionY);
#endif

	//CHECK HURT EVENTS: //todo
	/*{
		//CommandEcho(sprintf_z("xTile %d, yTile %d", (PositionX)&31, (PositionY)&31));
		int xTiles[2];
		int yTiles[2];

		bool doHitPlayer = false;
		{
			px = ((PositionX) - 12);
			py = (PositionY) - ((gravDir == 1) ? 4 : 16);//py = ((PositionY)-4);

																  //1 when on the right side of tile (generally used first...)
																  //	corresponds to xTiles[0] and yTiles[1]
																  //3 when on the left side of tile
																  //	corresponds to xTiles[1] and yTiles[1]
																  //0 or 2 when inside hurt tile
																  //  corresponds to yTiles[0]
			xTiles[0] = px / 32;		//(playerXTile-12)/32
			xTiles[1] = (px + 24) / 32;	//(playerXTile+12)/32

			yTiles[0] = py / 32;
			yTiles[1] = py / 32 + 1;

			for (int i = 0; i < 4; i++) {
				int xTile = xTiles[i / 2];
				int yTile = yTiles[i & 1];
				if (GetEvent(xTile, yTile) == areaHURT &&
					(!GetEventParam(xTile, yTile, 0, 4) ||	//default event
						GetEventParam(xTile, yTile, 0, 1)))		//top is set
				{
					for (ty = 1; ty < 32; ty++) {
						check = gameState.MaskedHLine(px, py + ty, 24);
						if (check && *tileAttr == areaHURT) {
							//CommandEcho(sprintf_z("top: hit player on %d", i));
							doHitPlayer = true;
							break;
						}
					}
					break;
				}
			}
		}

		{
			//Tframe *frame = &frames[curFrame];
			//ry=PositionY+((frame->hotSpotY - frame->gunSpotY));
			//CommandEcho(sprintf_z("frame width %d, height %d", frame->width, frame->height));
			//CommandEcho(sprintf_z("frame y %d", frame->hotSpotY - frame->coldSpotY));
			//CommandEcho(sprintf_z("frame hot %d, cold %d, gun %d", frame->hotSpotY, frame->coldSpotY, frame->gunSpotY));
			//CommandEcho(sprintf_z("player mask height %d, frame height %d", GetObjectMaskHeight((TgameObj*)play), frame->height));
		}

		if (!doHitPlayer) {
			px = ((PositionX) - 12);
			//py = ((PositionY)+6);	//not sure if I should use -4 or +4...
			py = ((PositionY) + 4);
			//special stuff for bottom hurt events

			xTiles[0] = px / 32;
			xTiles[1] = (px + 24) / 32;

			yTiles[0] = py / 32;
			yTiles[1] = py / 32 - 1;

			for (int i = 0; i < 4; i++) {
				int xTile = xTiles[i / 2];
				int yTile = yTiles[i & 1];
				if (GetEvent(xTile, yTile) == areaHURT &&
					GetEventParam(xTile, yTile, 1, 1))		//bottom is set
				{
					//for (ty = 1; ty < 32; ty++) {
					for (ty = 0; ty < 32; ty++) {
						check = gameState.MaskedHLine(px, py - ty, 24);
						if (check && *tileAttr == areaHURT) {
							//CommandEcho(sprintf_z("bottom: hit player on %d", i));
							doHitPlayer = true;
							break;
						}
					}
					break;
				}
			}
		}

		if (!doHitPlayer) {
			px = ((PositionX) - 12);
			//px = ((PositionX)-12+25);
			py = ((PositionY) - 4);

			xTiles[0] = px / 32;
			xTiles[1] = px / 32 + 1;
			//xTiles[0] = (px+25)/32;
			//xTiles[1] = (px+25)/32-1;

			yTiles[0] = py / 32;
			yTiles[1] = (py + 20) / 32;

			for (int i = 0; i < 4; i++) {
				int xTile = xTiles[i / 2];
				int yTile = yTiles[i & 1];
				if (GetEvent(xTile, yTile) == areaHURT &&
					GetEventParam(xTile, yTile, 2, 1))		//left is set
				{
					for (int tx = 0; tx < 26; tx++) {	//either 8 or 9...
														//check = CheckVLine(px+25, py, 20);
						int xPixel = px + tx;
						check = gameState.MaskedVLine(xPixel, py, 20);
						//CommandEcho(sprintf_z("check %d", check));
						if (check > 20) {
							check = 0;
						}
						if (check && *tileAttr == areaHURT) {
							xTile = xPixel / 32;
							//yTile = py/32;
							yTile = (py + check - 1) / 32;	//should work
							if (GetEventParam(xTile, yTile, 2, 1)) {	//must have same param
																		//CommandEcho(sprintf_z("left: hit player on %d", i));
								doHitPlayer = true;
								break;
							}
							else {
								//CommandEcho(sprintf_z("not a left hurt event on %d", i));
							}
						}
					}
					break;
				}
			}
		}

		if (!doHitPlayer) {
			px = ((PositionX) + 12);
			//px = ((PositionX)-12-1);
			py = ((PositionY) - 4);

			xTiles[0] = px / 32;
			xTiles[1] = px / 32 - 1;
			//xTiles[0] = (px-1)/32;
			//xTiles[1] = (px-1)/32+1;

			yTiles[0] = py / 32;
			yTiles[1] = (py + 20) / 32;

			for (int i = 0; i < 4; i++) {
				int xTile = xTiles[i / 2];
				int yTile = yTiles[i & 1];
				if (GetEvent(xTile, yTile) == areaHURT &&
					GetEventParam(xTile, yTile, 3, 1))		//right is set
				{
					for (int tx = 0; tx < 26; tx++) {
						//check = CheckVLine(px-1, py, 20);
						int xPixel = px - tx;
						check = gameState.MaskedVLine(xPixel, py, 20);
						//CommandEcho(sprintf_z("check %d", check));
						if (check > 20) {
							check = 0;
						}
						if (check && *tileAttr == areaHURT) {
							xTile = xPixel / 32;
							//yTile = py/32;
							yTile = (py + check - 1) / 32;	//should work
							if (GetEventParam(xTile, yTile, 3, 1)) {	//must have same param
																		//CommandEcho(sprintf_z("right: hit player on %d", i));
								doHitPlayer = true;
								break;
							}
							else {
								//CommandEcho(sprintf_z("not a right hurt event on %d", i));
							}
						}
					}
					break;
				}
			}
		}

		if (doHitPlayer) {
			HitPlayerLocal(playerID, 0, true);
		}
	}*/
}
void Bunny::DoZoneDetection()
{
}
void Bunny::ProcessAction()
{
}
void Bunny::AdjustViewpoint(GameState& gameState) const
{
	gameState.CenterCamera(PositionX, PositionY);
}
void Bunny::Behave(GameState& gameState)
{
	GetInput(gameState.Keys);
	ProcessInput();
	DoLandscapeCollision(gameState);
	DoZoneDetection();
	ProcessAction();
	AdjustViewpoint(gameState);
}
