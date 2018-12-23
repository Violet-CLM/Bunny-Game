#include "Misc.h"
#include "Bunny.h"
#include "BunnyMisc.h"
#include "BunnyObjectList.h"
#include "BunnyVersionDependentStuff.h"

Bunny::Bunny(ObjectStartPos & objStart, int characterIndex, Player& player) : BunnyObject(objStart), SpeedX(0), platformType(PlatformTypes::None), AccelerationX(0), SpeedY(0), AccelerationY(0), playerID(&player - Players.data()), platform_relX(0), platform_relY(0), freeze(0), invincibility(0), airBoard(0), helicopter(0), helicopterTotal(0), specialJump(0), dive(0), lastDive(0), fire(0), lastFire(0), lastDownAttack(0), hit(0), DirectionKeyX(0), DirectionKeyY(0), moveSpeedX(0), moveSpeedY(0), fixScrollX(0), quakeX(0), warpCounter(0), frogMorph(0), bossActive(0), vPole(0), swim(0), stop(0), stoned(0), stonedLen(0), spring(0), specialMove(0), slope(0), shiftPositionX(0), runDash(0), run(0), lastRun(0), rolling(0), quake(0), platform(0), ledgeWiggle(0), lastSpring(0), lastJump(0), idleTime(0), hPole(0), hang(0), vine(0), goUp(0), goRight(0), goLeft(0), goDown(0), goFarDown(0), fly(0), fixStartX(0), downAttack(DOWNATTACKLEN), charCurr(0), beMoved(0), sugarRush(0), sucked(0), shieldType(0), shieldTime(0), morph(0), flicker(0), fixAnim(false), frameCount(0), animSpeed(0), warpFall(0), warpArea(0), viewSkipAverage(0), skid(0), pushObject(0), push(0), poleSpeed(0), fireAngle(0), lookVP(0), lookVPAmount(0), lift(0), lastPush(0), lastLookVP(0), idleTrail(0), idleExtra(0), idleAnim(0), fireDirection(0)
{
	//the order here is important; Bunny::PlayerProperties::Object should remain nullptr so that operator= can be called in the other direction at the end of the level
	PlayerProperties = player;
	player.Object = this;

	PlayerProperties.CharacterIndex = characterIndex;
	AssignAnimation(RabbitAnims::STAND);
	DetermineFrame(1);
	DirectionX = DirectionY = 1;
	ObjectType = BunnyObjectType::Player;
	CollisionShapes.emplace_back(18,32);
	MakeNormal(13);
	LightType = LightType::Player; //partially distinct from Normal, partially not
	LightingTarget = HostLevel.StartLight;

	Health = START_HEALTH;
}
void Bunny::EatFood() {
	if (((PlayerProperties.Food += 1) % 100) == 0) {
		sugarRush = 20*AISPEED;
		//todo sound effects
	}
}
bool Bunny::Hurt(unsigned int damage)
{
	if (hit || flicker || invincibility || sugarRush || downAttack < DOWNATTACKWAIT) // || GameGlobals->level.finish
		return false;
	//todo bird
	//todo shield
	{
		Health -= damage;
		hit = AISPEED;
		idleTime = 0;
		//todo sound effects
	}
	return true;
}
Bunny::AttackTypes Bunny::GetAttackType(bool frozenEnemy) const {
	if (
		(PlayerProperties.CharacterIndex == char2JAZZ && specialMove > 16) ||
		(PlayerProperties.CharacterIndex == char2SPAZ && specialMove > 10) ||
		(PlayerProperties.CharacterIndex == char2LORI && specialMove > 1)
	)
		return AttackTypes::SpecialAttack;
	if (sugarRush)
		return AttackTypes::SugarRush;
	if (downAttack < DOWNATTACKLEN && SpeedY > 6)
		return AttackTypes::Buttstomp;
	if (frozenEnemy && abs(SpeedX) > 12)
		return AttackTypes::RunThroughFrozenEnemy;
	return AttackTypes::NotAttacking;
}
void Bunny::HitEnemyUsingAttackType(AttackTypes attackType) {
	switch (attackType) {
		case AttackTypes::SpecialAttack: //only for bosses
			DisableSpecialMove();
			break;
		case AttackTypes::SugarRush:
		case AttackTypes::Buttstomp:
			downAttack = DOWNATTACKLEN; //turn it off!
			SpeedY = SpeedY / -2 -8;
			AccelerationY = 0;
			DisableSpecialMove();
			AddToInvincibilityDuration(-AISPEED);
			break;
		case AttackTypes::RunThroughFrozenEnemy:
			AccelerationX = 0;
			SpeedX /= -2;
			SpeedY = -6;
			beMoved = AISPEED / 4;
			AddToInvincibilityDuration(-10); //shorter because less traditional
			break;
	}
}
void Bunny::AddToInvincibilityDuration(int addDuration) {
	if (addDuration > 0) invincibility = abs(invincibility) + addDuration;
	else if (invincibility <= 0) invincibility += addDuration;
	else invincibility -= addDuration;
}
void Bunny::DisableSpecialMove() {
	if (specialMove) {
		specialMove = 0;
		if (PlayerProperties.CharacterIndex == char2SPAZ) { ////don't kick bosses forever
			SpeedX = DirectionX << 4;
			SpeedY = 0;
			beMoved = 30;
			lastJump = 0;
		}
	} 
}

void Bunny::GetInput(const KeyStates& keys) {
	//todo base all these codes on JAZZ2.CFG
	KeyDown = keys.Key(sf::Keyboard::Down);
	KeyUp = !KeyDown && keys.Key(sf::Keyboard::Up);
	KeyRight = keys.Key(sf::Keyboard::Right);
	KeyLeft = !KeyRight && keys.Key(sf::Keyboard::Left);
	KeyFire = keys.Key(sf::Keyboard::Space);
	KeySelect = keys.Key(sf::Keyboard::Return);
	KeyRun = keys.Key(sf::Keyboard::LShift) || keys.Key(sf::Keyboard::RShift); //todo capslock
	KeyJump = keys.Key(sf::Keyboard::LControl) || keys.Key(sf::Keyboard::RControl);
	DirectionKeyX = KeyLeft ? -1 : int(KeyRight);
	DirectionKeyY = KeyUp ? -1 : int(KeyDown);
}

void Bunny::ProcessInputNoAirboard() {
	if (fly <= 1 /*|| gameObjects[0][fly - 1].behavior == copter*/) { //todo copter
		/*if (swim) { //todo swimming
			downAttack = DOWNATTACKLEN;
			runDash = lastRun = idleTime = 0;
			if (move->DirectionKeyX) {
				int xSpeed = xSpeed;
				if (move->DirectionKeyX & 0x8) { //left
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
		else*/ /*if (!(helicopter && characterProfile[PlayerProperties.CharacterIndex].helicopterXSpeed))*/ { //not swimming, not controlled by helicopter xSpeed //todo? character profiles
			if (runDash >= 0 && !beMoved) {
				if (DirectionKeyX) {
					runDash = 0;
					if (hit) {
						//do nothing
					} else if (run) {
						if (DirectionKeyX > 0)
								AccelerationX = 0.366210938f;
						else
							AccelerationX = -0.366210938f;
					} else if (DirectionKeyX > 0)
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
				if (getPlayerVarSettingLocal(pvANTIGRAV)) { //it might be better to do the job inside the cheshire2 code instead of here, but this is easier
					flyingObject->yPos -= flyingObject->ySpeed * 2;
					flyingObject->DirectionX = 0x40;
					newYSpeed = (40 * FIXMUL) - flyingObject->yPos + yPos;
				}
				else {
					flyingObject->DirectionX = 0;
					newYSpeed = (40 * FIXMUL) - yPos + flyingObject->yPos;
				}
				ySpeed = newYSpeed;
				xAcc = 0;
				yAcc = 0;
				ySpeed = newYSpeed - ((newYSpeed <= 0) ? 8192 : 24576);
			}
			else {
				flyingObject->state = sDONE; //I think this is right
				flyingObject->DirectionX = 0;
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
			;//PlaySample(PositionX, PositionY, (PlayerProperties.CharacterIndex != char2LORI) ? sCOMMON_JUMP : (RandFac(3) + sLORISOUNDS_LORIJUMP), 40, 0);
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
			//tCharacterProfile* profile = &characterProfile[PlayerProperties.CharacterIndex]; //todo? character indices
			if (PlayerProperties.CharacterIndex == char2SPAZ) {// profile->airJump == airjumpSPAZ) { //change, -ish
				if (specialJump < 1) { //profile->doubleJumpCountMax) {
					++specialJump;
					//if (onWallClimb()) //this part can (and nearly must) be done even if you're buttstomping
					//	PositionY -= 4;
					if (downAttack >= DOWNATTACKLEN) { //changed
						SpeedX = 0;// (DirectionX >= 0) ? profile->doubleJumpSpeedX : -profile->doubleJumpSpeedX;
						SpeedY = -8;// profile->doubleJumpSpeedY;
						spring = 0;
						PlaySampleAtObject(Common, UP);
					}
				}
			} else { //if (profile->airJump == airjumpHELICOPTER) { //Jazz or Lori
				{//if (helicopterTotal < profile->helicopterDurationMax) {
					helicopter = AISPEED;
					lastJump = 15; //was: 30
				}
			}
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
			liftlastSpring = gameTicks;
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
			if (PlayerProperties.CharacterIndex != char2LORI)
				PlaySample(Common, JUMP, sf::Vector2f(PositionX, PositionY), 40);
			else
				PlaySample(LoriSounds, LORIJUMP + RandFac(3), sf::Vector2f(PositionX, PositionY), 40);
			SpeedY = -10 - (abs(SpeedX) / 4); //jumpSpeed = -10
		}
		spring = 0;
		lastJump = 0;
		return;
	}
	else {
		if (!(((hang == 2) && (SpeedY != 0 || lastJump <= 2)) || hang == 0 || SpeedY != 0)) { //was <= 8, but that was more sensitive than needed just for preventing jumping up through hooks
			if (PlayerProperties.CharacterIndex != char2LORI)
				PlaySampleAtObject(Common, JUMP);
			else
				HostLevel.StartSound(AnimSets::Lori, Samples::sLoriSounds_LORIJUMP + RandFac(3), sf::Vector2f(PositionX, PositionY), 40); //no need for GetVersionSpecificAnimationID with AnimSets::Lori, because it's not in 1.23 in the first place
			SpeedY = -10;
			helicopterTotal = specialJump = 0;
			//if (!getPlayerVarSettingLocal(pvANTIGRAV)) //todo no fire zones
				PositionY -= 4;
			//else
			//	PositionY += 4;
			spring = 0;
		}
	}
	lastJump = 0;
}
void Bunny::ProcessInputStuffWithFlyAndSwim() {
	if (swim) {
		if (DirectionKeyY) {
			if (run == 0) {
				AccelerationY = DirectionKeyY / 8.f;
				if (SpeedY >= -2) {
					if (SpeedY > 2)
						SpeedY = 2;
				}
				else {
					SpeedY = -2;
				}
			}
			else {
				AccelerationY = DirectionKeyY / 4.f;
				if (SpeedY >= -4) {
					if (SpeedY > 4)
						SpeedY = 4;
				}
				else
					SpeedY = -4;
			}
		}
		else {
			AccelerationY = 0;
			if (SpeedY > 2)
				SpeedY = 2;
		}
	}
	else if (!fly) {
		if (helicopter <= 0) {// || helicopterTotal >= characterProfile[PlayerProperties.CharacterIndex].helicopterDurationMax) { //todo? character profiles
			helicopterTotal = helicopter = 0;
			AccelerationY = 0;
		}
		else {
			--helicopter;
			++helicopterTotal;
			//loopSample = PlayLoopSample(xPos, yPos, sCOMMON_HELI1, 25, 32000, loopSample); //todo sample
			const float maxSpeedY = 1;// characterProfile[PlayerProperties.CharacterIndex].helicopterSpeedY; //todo? character profiles
			if (maxSpeedY >= 0) {
				if (SpeedY >= 0) {
					AccelerationY = maxSpeedY / 128;
					if (SpeedY > maxSpeedY)
						SpeedY = maxSpeedY;
				}
				else
					AccelerationY = 0.375f;
			}
			else {
				if (SpeedY <= 0) {
					AccelerationY = maxSpeedY / 128;
					if (SpeedY < maxSpeedY)
						SpeedY = maxSpeedY;
				}
				else
					AccelerationY = -0.375f;
			}
			/*if (int maxXSpeed = characterProfile[PlayerProperties.CharacterIndex].helicopterXSpeed) {
				if (DirectionX < 0) maxXSpeed = -maxXSpeed;
				xAcc = maxXSpeed / 32;
				if (maxXSpeed >= 0) {
					if (xSpeed > maxXSpeed)
						xSpeed = maxXSpeed;
				}
				else {
					if (xSpeed < maxXSpeed)
						xSpeed = maxXSpeed;
				}
			}*/ //todo? character profiles
		}
	}
	else { //fly
		downAttack = DOWNATTACKLEN; //what does LEN stand for, anyhow? Length, as in duration? Maybe...
		helicopter = 0; //otherwise you get weird AccelerationY interactions
		if (fly == -1) { //airboard
			if (DirectionKeyY) {
				if (run == 0) {
					AccelerationY = 0.5f * DirectionKeyY;
					if (SpeedY < 0)
						AccelerationY -= 0.5f;
					if (SpeedY < -2)
						SpeedY = -2;
					else if (SpeedY > 2)
						SpeedY = 2;
				}
				else {
					AccelerationY = 0.75f * DirectionKeyY;
					if (SpeedY < 0)
						AccelerationY -= 0.5f;
					if (SpeedY < -4)
						SpeedY = -4;
					else if (SpeedY > 4)
						SpeedY = 4;
				}
			}
			else {
				AccelerationY = -0.109375f;
				if (SpeedY > 1) {
					SpeedY = 1;
				}
			}
		}
		else { //fly carrot
			if (fly < 1) //<-1... not sure what this would be?
				;// loopSample = PlayLoopSample(xPos, yPos, sCOMMON_HELI1, 25, 32000, loopSample); //todo sample
			if (!(DirectionKeyY)) {
				if (SpeedY >= 0)
					AccelerationY = -0.0625f;
				else
					AccelerationY = -0.125f;
			}
			if (DirectionKeyY) {
				SpeedY = float(run == 0 ? DirectionKeyY : DirectionKeyY * 2);
				if (SpeedY < 0)
					AccelerationY -= 0.25f;
			}
		}
	}
	if (DirectionKeyY > 0 && hang != 0)
		SpeedY = 4;
	if (DirectionKeyY || DirectionKeyX || KeyJump || KeySelect || runDash > 0) {
		fixAnim = 0;
		idleTime = 0;
	}
	else
		++idleTime;
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
		//if (move->fire && getPlayerVarSettingLocal(pvNOFIRE)) //todo no fire zones
			//move->fire = 0;
		/*if (GameGlobals->gameMode == GM_RACE && !IsLocalGameOrServer() && serverPlusVersion < PLUSVER_5_0) { //this countdown is in entirely the wrong spot, if you ask me //todo? race mode
			int currentGameTick = gameTicks;
			if (currentGameTick == 140 || currentGameTick == 210 || currentGameTick == 280)
				;//PlaySample(0, 0, sCOMMON_BELL_FIRE2, 0, 0);
			if (currentGameTick < 350)
				immobilized = true; //though I guess this part makes some sense
			else if (currentGameTick == 350) {
				lastLapFinish = 350;
				;//PlaySample(0, 0, sCOMMON_BELL_FIRE, 0, 0);
			}
		}*/
		immobilized |= (specialMove > 10);
		if (immobilized) {
			DirectionKeyX = DirectionKeyY = 0;
			KeyFire = KeyJump = KeyRun = false;
			lastFire = 800;
		}
		/*else if (fly > 1 && gameObjects[0][fly - 1].load == aCHESHIRE2)//todo cheshire cats
			*(BYTE *)move = 0;*/ 
}
	if (stonedLen > 0) {
		DirectionKeyX = !DirectionKeyX; DirectionKeyY = !DirectionKeyY;
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
				lastSpring = gameTicks;
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
		//return ProcessBirdInput(move);
	LowerToZero(frogMorph);
	LowerToZero(beMoved);
	//if (PlayerProperties.CharacterIndex == char2FROG)
		//return ProcessFrogInput(move);
	if (hang == 2) { //hook, not vine
		AccelerationX = 0;
		SpeedX = 0;
		if (DirectionKeyX < 0)
			DirectionX = -1;
		else
			DirectionX = 1;
		DirectionKeyX = 0;
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
		if (airBoard == 2) //todo sounds
			PlaySampleAtObject(Common, AIRBTURN2, 100, 16537);
		else if (airBoard == 12)
			PlaySampleAtObject(Common, AIRBTURN, std::min(48, int(abs(SpeedX)) + 32), 16537);
		if (!(DirectionKeyX)) {
			AccelerationX = 0;
			SpeedX = 31 * SpeedX / 32;
			if (airBoard < 14) {
				if (--airBoard <= 0) {
					airBoard = 14;
					DirectionX = -DirectionX;
				}
			}
		}
		else if (DirectionKeyX < 0) { //left
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
	if (!airBoard && DirectionKeyX) {
		if (DirectionKeyX < 0)
			DirectionX = -1;
		else
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
		DirectionKeyX = 0;
		DirectionKeyY = 0;
		if (run > 0)
			if ((run -= 4) < 0)
				run = 0;
	}
	else {
		{
			if (hang || vine) {
				if (DirectionKeyY <= 0 && goDown)
					downAttack = (DOWNATTACKLEN + AISPEED);
				else
					downAttack = DOWNATTACKLEN; //to prevent the player from buttstomping next tick if keyDown is still pressed
			}
			else if (goDown) {
				if (DirectionKeyY > 0) {
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
		if (KeyRun){// && characterProfile[PlayerProperties.CharacterIndex].canRun) { //changed //todo? character profiles
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
						PlaySampleAtObject(Ammo, BULFL2);
						if (runDash > 80)
							runDash = runDash = 80;
						SpeedX = (DirectionX * runDash) / 4.f;
						AccelerationX = SpeedX / 32.f;
						runDash = 4 * (-250 - runDash);
					}
				}
			}
			else if ((runDash = ++runDash) & 3)
				PlaySampleAtObject(Common, REVUP, 20);
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
			ProcessSelectInput(); //todo!

		ProcessInputJumpFallStuff();
		ProcessInputStuffWithFlyAndSwim(); //todo!
	}
}

void Bunny::ProcessFireInput()
{
	if ( SpeedX || SpeedY || !KeyUp || (fireType == Weapon::Blaster && shieldTime > 0) )
		fireDirection = 0;
	else
		fireDirection = 8;
	if ( fireType != Weapon::Blaster || shieldTime <= 0 || lastFire < 8 ) {
		if ( fireType == Weapon::Seeker || fireType == Weapon::RF || fireType == Weapon::TNT ) {
			if ( !fireHold ) {
				fire = 1;
				lastFire = -1;
				fireHold = true;
			}
		} else {
			if ( lastFire >= PlayerProperties.FireSpeed ) {
				fire = 1;
				lastFire = -1;
			}
		}
	} else {
		fire = 1;
		lastFire = -1;
	}
}
void Bunny::ProcessSelectInput()
{
	if ( KeyFire && downAttack >= DOWNATTACKLEN ) {
		ProcessFireInput();
	} else {
		fireHold = false;
		const int diminishedFireSpeed = PlayerProperties.FireSpeed - 5;
		if ( lastFire < diminishedFireSpeed )
			lastFire = diminishedFireSpeed;
	}
	++lastFire;
	if ( KeySelect ) {
		if ( !WasPressingKeySelectLastGameTick ) {
			WasPressingKeySelectLastGameTick = true;
			while (true) {
				if (++fireType >= Weapon::LAST) { fireType = Weapon::Blaster; break; } //blaster is infinite
				if (PlayerProperties.Ammo[fireType]) { break; }
			}
		}
	} else
		WasPressingKeySelectLastGameTick = false;
}

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
	they only scan in the DirectionX that you're facing+going in.
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
				;//PlaySample(PositionX, GameGlobals->level.waterLevel, sCOMMON_WATER, 0, 0);
				helicopter = 0; //violet. coptering into water made it impossible to move up from ground
									  // also, isn't there a bug where you can't fly up off a monitor if
									  // you have an airboard? that should get fixed!
			}
			swim = 1;
			fly = 0;	//to be sure
			if ((gameTicks & 63) < 31 && RandFac(7) == 0) {
				AddObject(PositionX + DirectionX * 16, PositionY - 8, aBUBBLE, 0);
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

	if (true) {// if (IsRabbit(PlayerProperties.CharacterIndex)) {	//todo !BIRD  and !FROG!!!!!!!!
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
					if (PlayerProperties.CharacterIndex == char2SPAZ)
						PlaySampleAtObject(SpazSounds, YAHOO);
					else if (PlayerProperties.CharacterIndex == char2LORI) //JJ2+ addition
						PlaySampleAtObject(LoriSounds, LORIFALL);
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
		else if (PlayerProperties.CharacterIndex == char2FROG) {

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
	if (hPole < -5 && vPole < -5) for (int stepY = 0; stepY < 2; stepY++) {
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
				/*if (PlayerProperties.CharacterIndex != char2FROG)*/ AccelerationY = -0.5f; //todo frog?
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
		SolidPlayers(&newPositionX, &newPositionY);
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

	OldSpeedX = SpeedX;
	OldSpeedY = SpeedY;
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
	if (!fly) {// && PlayerProperties.CharacterIndex != char2FROG) { //todo frog
		for (ty = (gravDir == 1 ? -8 : 11); ty; ty += gravDir) {
			bool lookingForHookOrVinePixels = false;
			for (int x = 0; x < 24; ++x)
				if (gameState.MaskedPixel(px + x, py + ty, tileAttr)) {
					if (tileAttr.ID == EventIDs::VINE || tileAttr.ID == EventIDs::HOOK) {
						lookingForHookOrVinePixels = true;
					}
					else {
						lookingForHookOrVinePixels = false;
						break;
					}
				}
			if (lookingForHookOrVinePixels) {
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
						PlaySampleAtObject(Common, FOEW3, 0, 30000);
					}
				}

				newPositionY = float(gravDir * 9 + py + ty);

				mSpeedY = newPositionY - PositionY;
				SpeedY = 0;
				AccelerationY = 0;
				break;
			}
		}	//for ty
	} //if not a bird/flier: falling

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
			//particular DirectionX

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

			checkY = std::max(28 + int(abs(SpeedX)), 32);

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
	if ((tileAttr.ID == EventIDs::VINE || tileAttr.ID == EventIDs::HOOK) && !hang/* && PlayerProperties.CharacterIndex != char2FROG*/) {
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

	if (abs(OldSpeedX - SpeedX) > 4) {
		quakeX = abs(OldSpeedX - SpeedX) / 1.25f;
	}

	if (!goDown && !fly && abs(OldSpeedY - SpeedY) > 3) {
		PlaySampleAtObject(Common, LANDPOP);

		if (downAttack == DOWNATTACKLEN) {
			//			AddExplosion(PositionX, PositionY+20, AnimBase[mAMMO]+mAMMO_BOOM1);
			//			quakeY = 5+abs(OldSpeedY - SpeedY);
			//			quake = -14;//-quakeY;
			quake = -AISPEED / 4;
		}
		/*else if (abs(OldSpeedY - SpeedY) > 5) { //todo explosions
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
void Bunny::DoZoneDetection(Event& curEvent, unsigned int gameTicks)
{
	const int currentTilePosition = (int(PositionX / TILEWIDTH) << 16) | int(PositionY / TILEHEIGHT);
	switch (curEvent.ID) {
	case EventIDs::TEXT: {
		const char* const str = HostLevel.HelpStrings[curEvent.GetParameter(0, 4)];
		if (str == HelpString && HelpStringCounter < 0)
			break;
		HelpString = str;
		HelpStringCounter = -6*AISPEED;
		if (curEvent.GetParameter(8, 1)) //vanish
			curEvent.ID = 0;
		break;
	}

	/*case aWATERBLOCK:
		if (lastEvent != aWATERBLOCK) {
			calc = (PositionY & 0xffe00000) + (curEvent.GetParameter(0, -8) << 16);
			AddExplosion(PositionX, calc, AnimBase[mCOMMON] + mCOMMON_SPLASH);

			;//PlaySample(PositionX, calc, sCOMMON_WATER, 0, 0);
		}
		break;*/

	/*case aWATERLEVEL:
		calc = curEvent.GetParameter(0, 8);

		if (calc == 0) {
			GameGlobals->level.newWaterLevel = py * 32 * 65536 + 15 * 65536;
			//} else if (calc > GameGlobals->level.blockHeight && curEvent.GetParameter(8, 1)) {
			//	GameGlobals->level.newWaterLevel = 0x7FFFFFFF;
		}
		else {
			GameGlobals->level.newWaterLevel = calc * 32 * 65536;
		}

		if (curEvent.GetParameter(8, 1)) {	//INSTANT!
			GameGlobals->level.waterLevel = GameGlobals->level.newWaterLevel;
		}
		// violet
		waterLightMode = curEvent.GetParameter(9, 2);
		break;*/

	/*case aMORPHFROG:
		if (PlayerProperties.CharacterIndex != char2FROG) {
			;//PlaySample(PositionX, PositionY, sFROG_JAZZ2FROG, 0, 0);

			//added so that the local client tells other clients/server of a change in
			//characters. ChangeCharacter also always sets .fly to zero when morphing
			//from a bird.
			ChangeCharacter(char2FROG);

			frogMorph = 2 * AISPEED;
			fly = 0;
		}
		break;*/

	case EventIDs::SUCKERTUBE: {
		if (curEvent.GetParameter(16, 1) != (platformType == PlatformTypes::SuckerTube))
			break;

		warpFall = 0;
		downAttack = DOWNATTACKLEN; //turn it off!
		specialMove = 0;
		const int waitTime = (LastSuckerTube != &curEvent) ? curEvent.GetParameter(17, 3) : 0;
		LastSuckerTube = &curEvent;
		if (sucked >= 0 && waitTime) {
			sucked = -waitTime*AISPEED / 2; //negative sucked means time to wait
			SpeedX = SpeedY = platform_relX = platform_relY = 0;
			CenterInTile();
		} else { //go!
			if (curEvent.GetParameter(14, 1))// TrigSample, written by blur, uncommented by Violet
				PlaySampleAtObject(Common, BURN, 0, 50000); //perhaps not the ideal sound?

			SpeedX = curEvent.GetParameter(0, -7);
			SpeedY = (curEvent.GetParameter(7, -7)) * DirectionY;
			AccelerationX = 0;
			AccelerationY = 0;

			if (SpeedY && (!SpeedX || currentTilePosition != lastTilePosition))	//align!
				CenterInTile(true, false);
			if (SpeedX && (!SpeedY || currentTilePosition != lastTilePosition))
				CenterInTile(false, true);

			if (SpeedY < 0) {
				spring = 1;
				lastSpring = gameTicks;
			}

			beMoved = AISPEED / 4;
			sucked = AISPEED / 2;

			if (curEvent.GetParameter(15, 1)) { //noclip again
				platformType = PlatformTypes::SuckerTube;
				platform_relX = SpeedX;
				platform_relY = SpeedY*DirectionY;
				//playX->gravity = 0;
			} else {
				platformType = PlatformTypes::None;
				//playX->gravity = GameGlobals->level.gravity;
			}
		}
		break; }

	/*case areaLIMITXSCROLL: //beefed up by violet
		calc = curEvent.GetParameter(0, 20);
		if (calc == 0) calc = px; //default behavior
		if ((fixStartX & 1023) != (calc & 1023)) {
			//only when not done before (when standing still in
			//this event, scrolling already occurs)
			fixStartX = calc; //limit scrolling
			calc &= 1023;
			if ((calc != fixStartX) || (abs(calc - (viewStartX >> 21)) * 32 < *screenWidth)) fixScrollX = viewStartX;
			else fixScrollX = calc << 21;
		}
		break;*/

	case EventIDs::HPOLE:
		if (!fly) {// && IsRabbit(PlayerProperties.CharacterIndex)) { //todo frog, bird
			if (currentTilePosition != lastTilePosition || hPole < -5) {
				if (SpeedX > 0) {
					AccelerationX = SpeedX + 8;
					if (AccelerationX > 20) {
						AccelerationX = 20;
					}
				}
				else {
					AccelerationX = SpeedX - 8;
					if (AccelerationX < -20) {
						AccelerationX = -20;
					}
				}
				poleSpeed = AccelerationX;
				CenterInTile();
				spring = 0;
				hPole = AISPEED;
				vPole = 0;
				downAttack = DOWNATTACKLEN; //turn it off!
				specialMove = 0;
			}
		}
		break;


	case EventIDs::VPOLE:
		if (!fly) { // && IsRabbit(PlayerProperties.CharacterIndex)) {//todo frog, bird
			if (currentTilePosition != lastTilePosition || vPole < -5) {
				if (SpeedY > 0)
					SpeedY = 16;
				else
					SpeedY = -16;
				poleSpeed = SpeedY;
				CenterInTile();

				vPole = AISPEED;
				hPole = 0;
				downAttack = DOWNATTACKLEN; //turn it off!
				specialMove = 0;
			}
		}
		break;

	/*case areaTRIGGERZONE:
		calc = (px + py*((int)GameGlobals->level.blockWidth + 1)) & 0xffff;
		{
			//int duration;
			int testCalc = lastTilePosition & 0xffff;
			int testTrig = lastTilePosition >> 16;

			int triggerID = curEvent.GetParameter(0, 5);

			//if (calc != testCalc && triggerID != testTrig) {
			if (lastEvent != areaTRIGGERZONE || calc != testCalc || triggerID != testTrig) {
				int doSwitch = curEvent.GetParameter(6, 1);

				if (doSwitch) {
					triggers[triggerID] = !triggers[triggerID];
				}
				else {
					triggers[triggerID] = curEvent.GetParameter(5, 1);
				}
				//lastTilePosition = testCalc+(testTrig<<16);	//holy mother of god!!!
				lastTilePosition = calc + (triggerID << 16);
			}
		}
		break;*/

	/*case areaNOFIREZONE: // one of violet's pet projects.
		if (AllowLocallyEnactedAdditions()) {
			int bitset = 1 << curEvent.GetParameter(2, 18);
			int oldBit = noGun & bitset;

			switch (curEvent.GetParameter(0, 2)) {
			case 1: // set off
				noGun &= ~bitset;
				break;
			case 2: // set on
				noGun |= bitset;
				break;
			case 3: // toggle
				if (lastTilePosition != currentTilePosition)
					noGun ^= bitset;
				break;
			case 0: //local
				noGun |= bitset << 16;
				break;
			}
			//special treatment cases
			if ((bitset == 2) && ((noGun & 2) != oldBit)) {// antigrav speed fix
				SpeedY *= -1;
				//DirectionKeyY *= -1;
				AddUpdatedPlayer_Client(playerID);
			}
			else if (bitset == 4 && (noGun & 4) != oldBit)
				AddUpdatedPlayer_Client(playerID);
		}
		else
			noGun |= 0x10000; //local noFire
		break;*/

	case EventIDs::SETLIGHT:
		LightingTarget = curEvent.GetParameter(0, 8) * 64 / 100;
		break;

	/*case areaDIMLIGHT: // dunno. can't find the right function to hack. been using this for debug stuff instead. (violet)
		break;*/

	case EventIDs::RESETLIGHT:
		LightingTarget = HostLevel.StartLight;
		break;

	/*case areaECHO:
		GameGlobals->level.echo = curEvent.GetParameter(0, 8);
		break;*/

	/*case areaFLYOFF:
		if (fly == -1) {	//airboard floating off
			calc = AddObject(PositionX, PositionY, aBOUNCEONCE, 0);
			if (calc > 0) {
				gameObjects[0][calc].curAnim = AnimBase[mPICKUPS] + mPICKUPS_GETBOARD;
				gameObjects[0][calc].behavior = cAIRBFALL;
				((Omonster*)(&gameObjects[0][calc]))->SpeedX = SpeedX;
			}
		}
		fly = 0;
		if (charCurr == mBIRD) {
			ChangeCharacter(GetIndexFromPlayerCharacter(charOrig));
		}
		break;*/

	case EventIDs::WARPEOL:
	case EventIDs::ENDOFLEVEL:
			//todo waiting period
			HostLevel.ReplaceWithNewStage(Level::LoadLevel(HostLevel.NextLevel));
		/*if (!NetGlobals->isServer || GameGlobals->gameMode == GM_RACE) {
			GameEvent::EndOfLevel(px, py, playerID);
		}
		if (GameGlobals->gameMode == GM_TREASURE)
			CheckPlayerCoins(playerID, *maxScore, false, 255, false);*/
		break;

	/*case areaWARPEOL:
		if (!NetGlobals->isServer || GameGlobals->gameMode == GM_RACE) {
			GameEvent::WarpEOL(px, py, playerID);
		}
		if (GameGlobals->gameMode == GM_TREASURE)
			CheckPlayerCoins(playerID, *maxScore, false, 255, false);
		break;*/

	/*case areaWARPSECRET:
		if (!NetGlobals->isServer || GameGlobals->gameMode == GM_RACE) {
			GameEvent::WarpSecret(px, py, playerID);
		}
		break;*/

	/*case areaENDMORPH:
		if (charCurr != charOrig) {
			//added so that the local client tells other clients/server of a change in
			//characters. ChangeCharacter also always sets .fly to zero when morphing
			//from a bird.
			ChangeCharacter(GetIndexFromPlayerCharacter(charOrig));
			morph = -AISPEED / 2;
		}
		break;*/

	/*case areaWARP:
		if (lastTilePosition != currentTilePosition && curEvent.GetParameter(17, 1) == 0) ////don't make poor frog/bird players lose more and more coins every tick... also, if ShowAnim=1, use the object's code instead of this stuff
			tryToWarpToID(curEvent.GetParameter(0, 8), curEvent.GetParameter(8, 8), !!curEvent.GetParameter(16, 1), !!curEvent.GetParameter(18, 1));
		break;*/

	/*case areaROCKTRIGGER:
		//if (lastTilePosition != currentTilePosition) {
		triggerRock(curEvent.GetParameter(0, 8));
		//}
		break;*/

	/*case areaBOSSTRIGGER:
		if (!bossActive) {
			if (SoundGlobals->soundEnabled) {
				const string nonBossMusic = musicFilename;
				if (!LoadNewMusicFile((!curEvent.GetParameter(0, 1)) ? "boss1.j2b" : "boss2.j2b") && sugarRush > 0)
					MusicPlay();
				musicFilename = nonBossMusic;
			}

			bossActive = 1;

			if (sugarRush > 0)
				sugarRush = 0;	//turn it off... woa
		}
		break;*/
	}
	lastTilePosition = currentTilePosition;
}

inline void Bunny::CenterInTile(bool x, bool y)
{
	if (x)
		PositionX = float(int(PositionX) / TILEWIDTH) * TILEWIDTH + 15;
	if (y)
		PositionY = float(int(PositionY) / TILEHEIGHT) * TILEHEIGHT + 15;
}

void Bunny::PoleSamples() const {
	/*if (!SoundGlobals->soundEnabled)
		return;
	if (play.PlayerProperties.CharacterIndex != char2SPAZ) {
		;//PlaySample(play.xPos, play.yPos, sCOMMON_SWISH5 + RandFac(3), 0, 0);
		if (play.PlayerProperties.CharacterIndex == char2LORI)
			;//PlaySample(play.xPos, play.yPos, sLORISOUNDS_WEHOO, 0, 0);
	}
	else {
		;//PlaySample(play.xPos, play.yPos, sCOMMON_SWISH1 + RandFac(3), 0, 0);
		const int finalPoleSamples[4] = { sSPAZSOUNDS_WOOHOO, sSPAZSOUNDS_OHOH, sSPAZSOUNDS_HOHOHO1, 0 }; //version-dependent, so can't be const
		const int finalPoleSample = finalPoleSamples[RandFac(3)];
		if (finalPoleSample > 0)
			;//PlaySample(play.xPos, play.yPos, finalPoleSample, 0, 0);
	}*/
}

inline bool Bunny::UsesAnimation(int anim) const {
	return AnimID == RabbitAnimIDs[anim];
}
inline void Bunny::AssignAnimation(int anim) {
	AnimID = RabbitAnimIDs[anim];
}
#define ANIM_SPEED_MAX 256
inline void Bunny::AssignAnimation(int anim, int as) {
	AssignAnimation(anim);
	animSpeed = as;
}
inline void Bunny::AssignAnimation(int anim, int as, bool fa, int fi) {
	AssignAnimation(anim, as);
	frameCount = 0;
	FrameID = (fi >= 0) ? fi : GetFrameCount() + fi;
	fixAnim = fa;
}
void Bunny::AdjustRabbit(unsigned int gameTicks) {
	float absoluteSpeed;

	if (fixAnim)
		if ((abs(SpeedX) > 0.015625f && (UsesAnimation(RabbitAnims::FALLLAND) || UsesAnimation(RabbitAnims::BUTTSTOMPLAND)))
			||
			(!idleTime && AnimID == idleAnim))
			fixAnim = false;

	if (frogMorph) {
		idleTime = 0;
		if (!fixAnim) {
			if (UsesAnimation(RabbitAnims::FROG)) {
				frogMorph = 0;
				AssignAnimation(RabbitAnims::STAND, ANIM_SPEED_MAX, false);
			}
			else {
				AssignAnimation(RabbitAnims::FROG, 7, true);
			}
		}
	}
	/*else if (GameGlobals->level.finish == 1 && GameGlobals->level.finishCounter > 0x8000) {
		DirectionX = 1;
		fly = 0;
		if (hang) {
			PositionY += 4;
			SpeedY = 4;
		}
		else if (SpeedY < 0)
			SpeedY = 0;
		SpeedX = 0;
		AccelerationX = 0;
		AccelerationY = 0;
		if (!goDown) {
			if (!UsesAnimation(RabbitAnims::ENDOFLEVEL)) {
				AssignAnimation(RabbitAnims::ENDOFLEVEL, 6, true);
			}
			else {
				if (frameCount == 0) {
					if (FrameID == 1)
						;//PlaySample(PositionX - 64, PositionY, sCOMMON_REVUP, 128, 0);
					else if (FrameID == 10)
						;//PlaySample(PositionX + 64, PositionY, sAMMO_BULFL3, 127, 0);
				}
				if (!fixAnim) {
					if (NetGlobals->partyMode || *numPlayers != 1 && !IsSinglePlayerBasedGame()) { //todo end level stuff
						if (!MenuGlobals->menuCode)
							MenuGlobals->menuCode = quitWAIT;
					}
					else
						MenuGlobals->menuCode = quitLEVEL;
					AssignAnimation(RabbitAnims::ENDOFLEVEL, ANIM_SPEED_MAX, false, -1);
				}
			}
		}
	}
	else if (GameGlobals->level.finish == 2 && GameGlobals->level.finishCounter > 0x8000) {
		DirectionX = 1;
		fly = 0;
		if (hang) {
			PositionY += 4;
			SpeedY = 4;
		}
		else if (SpeedY < 0)
			SpeedY = 0;
		SpeedX = 0;
		AccelerationX = 0;
		AccelerationY = 0;
		if (!goDown) {
			if (!UsesAnimation(RabbitAnims::TELEPORT)) {
				fixAnim = 0;
				AssignAnimation(RabbitAnims::TELEPORT, 6, true);
			}
			else if (!fixAnim) {
				if (!IsSinglePlayerBasedGame()) { //todo end level stuff
					if (!MenuGlobals->menuCode)
						MenuGlobals->menuCode = quitLEVEL | quitWAIT;
				}
				else {
					MenuGlobals->menuCode = quitLEVEL;
				}
				warpCounter = 2;
				warpArea = 1;
				AssignAnimation(RabbitAnims::TELEPORT, ANIM_SPEED_MAX, false, -1);
			}
		}
	}
	else*/ //todo end level stuff
	if (warpArea) {
		if (warpCounter == 0) {
			fixAnim = 0;
			PlaySampleAtObject(Common, TELPORT1, 128);
			if (!fixAnim) {
				AssignAnimation(warpFall ? RabbitAnims::TELEPORTFALLTELEPORT : RabbitAnims::TELEPORT, 5, true);
			}
			warpCounter = 1;
		}
		else if (warpCounter == 1) {
			SpeedX = 0;
			SpeedY = 0;
			AccelerationX = 0;
			AccelerationY = 0;
			if (!fixAnim) {
				AssignAnimation(warpFall ? RabbitAnims::TELEPORTFALLTELEPORT : RabbitAnims::TELEPORT, ANIM_SPEED_MAX, false, -1);
				PositionX = WarpTargetPositionX;
				PositionY = WarpTargetPositionY;
				warpCounter = 2;
				viewSkipAverage = 1;
				//SetPlayerNoPredict(a1, true); //JJ2+ change; was onAdjustNormalPlayer_GetWarpTargetPosition //todo? network code
			}
		}
		else if (warpCounter != 2) {
			if (!fixAnim || (SpeedX = 0, SpeedY /= 2, AccelerationX = 0, AccelerationY = 0, !fixAnim)) {
				if (warpFall) {
					if (!goDown || helicopter) {
						warpFall = 0;
						warpArea = 0;
						warpCounter = 0;
					}
					else {
						AssignAnimation(RabbitAnims::TELEPORTFALLING, 5);
					}
				}
				else {
					AssignAnimation(RabbitAnims::TELEPORTSTAND, ANIM_SPEED_MAX, false, -1);
					warpArea = 0;
					warpCounter = 0;
				}
			}
		}
		else {
			warpFall = goDown && goFarDown;
			AssignAnimation(warpFall ? RabbitAnims::TELEPORTFALL : RabbitAnims::TELEPORTSTAND, 5, true);
			PlaySampleAtObject(Common, BUBBLGN1, 10);
			PlaySampleAtObject(Common, TELPORT2, 128, 22050);
			warpCounter = -1;
		}
	}
	else if (!Health) {
		SpeedX = 0;
		AccelerationX = 0;
		AccelerationY = 0;
		if (SpeedY < 0)
			SpeedY = 0;
		if (!UsesAnimation(RabbitAnims::DIE)) {
			AssignAnimation(RabbitAnims::DIE, 6, true);
			/*if (!finished) {
				if (!deathSequence)
					deathSequence = AISPEED * 5;
				for (int i = 0; i < MAXDISPLAYS; ++i) //ClearDisplayMessages
					display[i].clear();
				display[0].state = 7;
				strcpy_s(display[0].text, StrGlobals->strDeathTextList[RandFac(7)]);
				SoundGlobals->lastMusicVolume = SoundGlobals->musicVolume;
			}*/ //todo end level stuff
		}
		else {
			/*if (SoundGlobals->soundEnabled) {
				if (GameGlobals->localPlayers == 1) {
					SoundGlobals->musicVolume -= 4;
					if (SoundGlobals->musicVolume < 0)
						SoundGlobals->musicVolume = 0;
					Sound_UpdateVolume();
				}
				if (frameCount == 0) {
					if (PlayerProperties.CharacterIndex == char2JAZZ) {
						int samplePositionX = PositionX, sampleFrequency = 0, sampleVolume, sampleSample = 0; //todo samples
						switch (FrameID) {
						case 0:
							sampleVolume = 0;
							sampleSample = sCOMMON_PREEXPL1;
							break;
						case 2:
							;//PlaySample(PositionX, PositionY, sCOMMON_PLOPKORK, 20, 0);
							sampleVolume = 0;
							sampleSample = sCOMMON_BENZIN1;
							break;
						case 5:
							sampleVolume = 40;
							sampleSample = sCOMMON_BASE1;
							break;
						case 9:
							sampleVolume = 0;
							sampleSample = sCOMMON_DOWNFL2;
							break;
						case 11:
							sampleVolume = 50;
							sampleSample = sCOMMON_HEAD;
							break;
						case 12:
							sampleVolume = 128;
							sampleSample = sCOMMON_BASE1;
							samplePositionX -= 0x640000;
							break;
						case 14:
							sampleFrequency = 14000;
							sampleVolume = 128;
							sampleSample = sCOMMON_BASE1;
							samplePositionX += 0x640000;
							break;
						}
						if (sampleSample)
							;//PlaySample(samplePositionX, PositionY, sampleSample, sampleVolume, sampleFrequency);
					}
					else if (PlayerProperties.CharacterIndex == char2SPAZ) {
						int samplePositionX = PositionX, sampleFrequency, sampleVolume, sampleSample = 0; //todo samples
						switch (FrameID) {
						case 1:
							sampleFrequency = 0;
							sampleVolume = 0;
							sampleSample = sSPAZSOUNDS_BURP;
							break;
						case 4:
							sampleFrequency = 10000;
							sampleVolume = 40;
							sampleSample = sCOMMON_STRETCH;
							break;
						case 7:
							sampleFrequency = 0;
							sampleVolume = 0;
							sampleSample = sCOMMON_BENZIN1;
							break;
						case 14:
							;//PlaySample(PositionX - 0x640000, PositionY, sCOMMON_CUP, 50, 9000);
							sampleFrequency = 0;
							sampleVolume = 50;
							sampleSample = sCOMMON_STEAM;
							break;
						case 17:
							sampleFrequency = 8000;
							sampleVolume = 30;
							sampleSample = sCOMMON_CUP;
							samplePositionX -= 0x640000;
							break;
						}
						if (sampleSample)
							;//PlaySample(samplePositionX, PositionY, sampleSample, sampleVolume, sampleFrequency);
					}
					else if (PlayerProperties.CharacterIndex == char2LORI && FrameID == 1) {
						;//PlaySample(PositionX, PositionY, sLORISOUNDS_DIE1, 0, 0);
					}
				}
			}*/ //todo samples
			if (!fixAnim) {
				//SoundGlobals->musicVolume = SoundGlobals->lastMusicVolume; //todo samples
				AssignAnimation(RabbitAnims::DIE, ANIM_SPEED_MAX, false, -1);
				/*if (*numPlayers > 1 && GameGlobals->level.finish) {
					if (!MenuGlobals->menuCode)
						MenuGlobals->menuCode = quitLEVEL | quitWAIT;
				}
				else if (extraLives < 0) {
					if (!GameGlobals->gameMode) {
						deathSequence = 0;
						if (!MenuGlobals->menuCode)
							MenuGlobals->menuCode = quitDEATH | quitWAIT;
					}
				}
				else if (!finished || IsSinglePlayerBasedGame()) {
					deathSequence = 0;
					LoadStartGameText(a1);
					if (GameGlobals->localPlayers == 1) {
						ModMusicPause();
						if (strncmp(SoundGlobals->origMusicFilename, SoundGlobals->loadedMusicFilename, MAX_LEVEL_FILENAME_LENGTH)) {
							MusicUnload();
							LoadMusicFile(SoundGlobals->origMusicFilename);
						}
						else {
							SetMusicStart();
						}
						ModMusicResume();
						Sound_SlideVolume(1);
					}
					if (GameGlobals->gameMode == GM_SP) {
						ReinitObjects();
					}
					KillPlayer(a1);
					onSetStartPosition(a1); //contains JJ2+ change
					InitEvents(a1);
				}*/ //todo end level stuff
			}
		}
	}
	else if (hit > 0) {
		idleTime = 0;
		if (hit == AISPEED) {
			AssignAnimation(RabbitAnims::HURT, 4, true);
			if (abs(SpeedX) <= 2)
				SpeedX = -4.f * DirectionX;
			else
				SpeedX = SpeedX / -2 - (DirectionX << 17);
			if (SpeedY >= 0)
				SpeedY = -4;
			--hit;
		}
		else {
			if (!UsesAnimation(RabbitAnims::HURT) || fixAnim)
				--hit;
			else
				hit = 0;
			if (hit <= 0) {
				invincibility = AISPEED * -3;
				flicker = AISPEED * -3;
			}
		}
	}
	else if (hPole >= 0) { //not convinced this should be in Adjust
		if (hPole) { //still counting down
			PositionX = float((int(PositionX) & ~(TILEWIDTH-1)) + 15);
			PositionY = float((int(PositionY) & ~(TILEHEIGHT-1)) + 15);
			SpeedX = 0;
			AccelerationX = 0;
			SpeedY = 0;
			AccelerationY = 0;
		}
		else { //release!
			SpeedX = poleSpeed;
			PositionX += DirectionX << 2;
			beMoved = AISPEED;
			SpeedY = -1;
			rolling = AISPEED / 2;
			PoleSamples();
		}
		if (!fixAnim)
			AssignAnimation(RabbitAnims::HPOLE, std::max(1, hPole / 10));
		if (!frameCount && !FrameID)
			PlaySampleAtObject(Common, BIRDFLY, 0, 44000 - (hPole << 8));
	}
	else if (vPole >= 0) {
		if (vPole) {
			PositionX = float((int(PositionX) & ~(TILEWIDTH - 1)) + 15);
			PositionY = float((int(PositionY) & ~(TILEHEIGHT - 1)) + 15);
			SpeedX = 0;
			AccelerationX = 0;
			SpeedY = 0;
			AccelerationY = 0;
		}
		else {
			SpeedY = poleSpeed;
			if (SpeedY <= 0)
				PositionY -= 8;
			else
				PositionY += 8;
			rolling = beMoved = AISPEED / 2;
			SpeedX = 0;
			AccelerationX = 0;
			PoleSamples();
		}
		if (!fixAnim)
			AssignAnimation(RabbitAnims::VPOLE, std::max(1, vPole / 10));
		if (!frameCount && !FrameID)
			PlaySampleAtObject(Common, BIRDFLY, 0, 44000 - (vPole << 8));
	}
	else if (sucked) {
		ApproachZero(sucked);
		idleTime = 0;
		if (!fixAnim)
			AssignAnimation(RabbitAnims::ROLLING, 4);
		fire = 0;
		lastFire = (int)"HUHA";
	}
	else if (rolling > 0) {
		--rolling;
		idleTime = 0;
		if (!UsesAnimation(RabbitAnims::ROLLING) && !fixAnim)
			AssignAnimation(RabbitAnims::ROLLING, 4);
		fire = 0;
		lastFire = (int)"ARB";
	}
	else if (swim) {
		if (!fixAnim) {
			const int proposedAnimSpeed = std::max(4, int(10 - SpeedX / (SpeedX <= 0 ? -0.5f : 0.5f)));
			if (SpeedY >= -2) {
				if (SpeedY <= 2)
					AssignAnimation(RabbitAnims::SWIMRIGHT, proposedAnimSpeed);
				else if (UsesAnimation(RabbitAnims::SWIMDOWN) || UsesAnimation(RabbitAnims::SWIMTURN1))
					AssignAnimation(RabbitAnims::SWIMDOWN, proposedAnimSpeed);
				else
					AssignAnimation(RabbitAnims::SWIMTURN1, 7, true);
			}
			else if (UsesAnimation(RabbitAnims::SWIMUP) || UsesAnimation(RabbitAnims::SWIMTURN2))
				AssignAnimation(RabbitAnims::SWIMUP, proposedAnimSpeed);
			else
				AssignAnimation(RabbitAnims::SWIMTURN2, 7, true);
		}
	}
	/*else if (vine) {
		idleTime = 0;
		const Omonster& vine = (Omonster&)gameObjects[0][vine];
		PositionX = vine.PositionX;
		PositionY = vine.PositionY;
		SpeedX = vine.SpeedX;
		SpeedY = vine.SpeedY;
		int FrameID;
		if (vine.var4 >= 0) {
			DirectionX = 1;
			FrameID = int(vine.var1 / 3.14f) >> 3; //todo lori
		}
		else {
			DirectionX = -1;
			FrameID = int((256 - vine.var1) / 3.14f) >> 3;
		}
		AssignAnimation(RabbitAnims::SWINGINGVINE, ANIM_SPEED_MAX, false, min(max(FrameID, 0), 10));
	}*/ //todo swinging vines
	else if (fly) {
		if (!goUp || SpeedY > 0)
			spring = 0;
		if (fly > 1) { //some sort of object
			if (lastFire >= PlayerProperties.FireSpeed + 25) {
				if (!fixAnim)
					AssignAnimation(((PlayerProperties.CharacterIndex != char2SPAZ) && (idleTime % (AISPEED * 2) <= AISPEED)) ? RabbitAnims::HANGIDLE1 : RabbitAnims::HANGIDLE2, 7);
			}
			else if (fireDirection == 8) { //up
				if (lastFire > PlayerProperties.FireSpeed + 10) {
					fixAnim = 0;
					AssignAnimation(RabbitAnims::HANGFIREQUIT, 5);
				}
				else if (!fixAnim)
					AssignAnimation(RabbitAnims::HANGFIREUP, PlayerProperties.FireSpeed / 2, true);
			}
			else {
				if (lastFire > PlayerProperties.FireSpeed + 10) {
					fixAnim = 0;
					AssignAnimation(RabbitAnims::HANGINGFIREQUIT, 5);
				}
				else if (!fixAnim)
					AssignAnimation(RabbitAnims::HANGINGFIRERIGHT, PlayerProperties.FireSpeed / 2, true);
			}
		}
		else if (fly == -1) { //airboard
			if (!fixAnim)
				AssignAnimation(RabbitAnims::AIRBOARD, 6);
		}
		else { //fly carrot
			if (lastFire < PlayerProperties.FireSpeed + 20) {
				if (!fixAnim) {
					if (lastFire <= PlayerProperties.FireSpeed + 10)
						AssignAnimation(RabbitAnims::HELICOPTERFIRERIGHT, PlayerProperties.FireSpeed / 2);
					else
						AssignAnimation(RabbitAnims::HELICOPTERFIREQUIT, 5);
				}
			}
			else if (!fixAnim)
				AssignAnimation(RabbitAnims::HELICOPTER, 4);
		}
	}
	else if (specialMove) {
		if (PlayerProperties.CharacterIndex == char2JAZZ || PlayerProperties.CharacterIndex == char2SPAZ) {
			if (specialMove < 28)
				AssignAnimation(RabbitAnims::STATIONARYJUMPSTART, ANIM_SPEED_MAX, false, specialMove / 4);
			else if (specialMove < 70 && !fixAnim)
				AssignAnimation(RabbitAnims::STATIONARYJUMP, 4);
		}
		else if (PlayerProperties.CharacterIndex == char2LORI) {
			const int specificFrameIDInLorisAttack = 4 * GetFrameCount() - 4;
			if (specialMove >= specificFrameIDInLorisAttack) {
				beMoved = 0;
				specialMove = 0;
			}
			else {
				if (specialMove >= 14) {
					SpeedX -= SpeedX / 16;
				}
				else {
					SpeedX = specialMove * specialMove * DirectionX / 4.f;
				}
				beMoved = specificFrameIDInLorisAttack - specialMove;
				AssignAnimation(RabbitAnims::STATIONARYJUMPSTART, ANIM_SPEED_MAX, false, specialMove / 4);
			}
		}
	}
	else if (lift) {
		if (!fixAnim) {
			if (lift < 0x8000)
				AssignAnimation(RabbitAnims::LIFT, 12);
			else
				AssignAnimation(RabbitAnims::LIFTJUMP, 4, true);
		}
	}
	else if (slope && (absoluteSpeed = abs(SpeedX)) > 1.f) {
		const int runningAnimSpeed = 11 - int(absoluteSpeed);
		if (!fixAnim) {
			if (stonedLen) {
				AssignAnimation(RabbitAnims::LOOPY, 6);
			}
			else {
				AssignAnimation((absoluteSpeed >= 8 && run) ? RabbitAnims::RUN3 : (absoluteSpeed > 4) ? RabbitAnims::RUN2 : RabbitAnims::RUN1, runningAnimSpeed);
			}
		}
		else
			animSpeed = runningAnimSpeed;
	}
	else if (hang) {
		if (lastFire >= PlayerProperties.FireSpeed + 25) {
			if (SpeedX) {
				if (SpeedX < 1) { //issue #641
					if (SpeedX <= -1)
						SpeedX = -1 - sintable(FrameID << 6);
				}
				else {
					SpeedX = sintable(FrameID << 6) + 1;
				}
				if (run)
					SpeedX *= 2;
				if (!fixAnim)
					AssignAnimation(RabbitAnims::EARBRACHIATE, 4);
				if (frameCount == 0) {
					if (FrameID == 2 || FrameID == 7)
						PlaySampleAtObject(Common, STEP, 20, 8000);
				}
			}
			else if (!fixAnim) {
				if (PlayerProperties.CharacterIndex != char2JAZZ)
					AssignAnimation(RabbitAnims::HANGIDLE2, 7);
				else
					AssignAnimation((idleTime % (AISPEED * 2) <= AISPEED) ? RabbitAnims::HANGIDLE1 : RabbitAnims::HANGIDLE2, 7);
			}
		}
		else if (lastFire > PlayerProperties.FireSpeed + 10) {
			fixAnim = 0;
			AssignAnimation((fireDirection != 8) ? RabbitAnims::HANGINGFIREQUIT : RabbitAnims::HANGFIREQUIT, 5);
		}
		else {
			if (!fixAnim)
				AssignAnimation((fireDirection == 8) ? RabbitAnims::HANGFIREUP : RabbitAnims::HANGINGFIRERIGHT, PlayerProperties.FireSpeed / 2, true);
			SpeedX = 0;
			AccelerationX = 0;
		}
	}
	else if (spring) {
		idleTime = 0;
		if (!goUp || SpeedY > -1)
			spring = 0;
		if (lastFire >= PlayerProperties.FireSpeed + 20) {
			if (!fixAnim)
				AssignAnimation(RabbitAnims::SPRING, std::max(1, int(SpeedY + 10)));
		}
		else if (!fixAnim) {
			if (lastFire > PlayerProperties.FireSpeed + 10)
				AssignAnimation(RabbitAnims::JUMPFIREQUIT, 5);
			else
				AssignAnimation(RabbitAnims::JUMPFIRERIGHT, PlayerProperties.FireSpeed / 2);
		}
	}
	else if (!goDown) {
		if (!SpeedY) {
			helicopter = 0;
			if (lastFire <= PlayerProperties.FireSpeed + 10) {
				skid = 0;
				if (UsesAnimation(RabbitAnims::BUTTSTOMPLAND) || UsesAnimation(RabbitAnims::FALLLAND) || UsesAnimation(RabbitAnims::SKID2) || UsesAnimation(RabbitAnims::SKID3))
					fixAnim = 0;
				absoluteSpeed = abs(SpeedX);
				if (absoluteSpeed) {
					if (!stonedLen) {
						if (!fixAnim)
							AssignAnimation((absoluteSpeed > 0x40000) ? RabbitAnims::RUN2 : RabbitAnims::RUN1, 11 - int(absoluteSpeed));
					}
					else if (!fixAnim)
						AssignAnimation(RabbitAnims::LOOPY, 6);
				}
				else if (!fixAnim)
					AssignAnimation(stonedLen ? RabbitAnims::STONED : RabbitAnims::STAND, 6);
			}
			else {
				if (OldSpeedY > 2) {
					if (abs(SpeedX) < 0.5f) {
						idleTime = 0;
						if (!fixAnim) {
							if (lastDownAttack < 8) // __OFSUB__
								AssignAnimation(RabbitAnims::BUTTSTOMPLAND, 4, true);
							else
								AssignAnimation(RabbitAnims::FALLLAND, 2, true);
						}
					}
				}
				if (pushObject || (DirectionKeyX > 0 && !goRight) || (DirectionKeyX < 0 && !goLeft)) {
					if (++push >= 16 && !fixAnim) // __OFSUB__
						AssignAnimation(RabbitAnims::PUSH, 8);
				}
				else
					push = 0;
				if (AccelerationX || stonedLen) {
					skid = 0;
					if (UsesAnimation(RabbitAnims::SKID2) || UsesAnimation(RabbitAnims::SKID3))
						fixAnim = 0;
				}
				else if (abs(SpeedX) <= 1) { //__OFSUB__
					if (skid <= 20 || !UsesAnimation(RabbitAnims::SKID1))
						skid = 0;
					else
						AssignAnimation(RabbitAnims::SKID3, 3, true);
				}
				else {
					if (!fixAnim) {
						if (skid)
							AssignAnimation(RabbitAnims::SKID1, 6);
						else
							AssignAnimation(RabbitAnims::SKID2, 3, true);
					}
					++skid;
				}
			}
			if (DirectionKeyY <= 0 || AccelerationX) {
				dive = 0;
			}
			if (lastDive = gameTicks, ++dive > 4 && downAttack > 85) { //looking down
				lastLookVP = gameTicks;
				if (++lookVP > 30) {
					if (lookVPAmount < 0)
						lookVPAmount += 3;
					else if (lookVPAmount < 60)
						lookVPAmount += 1;
				}
				if (lastFire >= PlayerProperties.FireSpeed + 15) {
					AssignAnimation(RabbitAnims::DIVE, ANIM_SPEED_MAX, false, 1);
				}
				else if (!fixAnim) {
					if (lastFire < PlayerProperties.FireSpeed + 10) // __OFSUB__
						AssignAnimation(RabbitAnims::DIVEFIRERIGHT, PlayerProperties.FireSpeed / 2);
					else
						AssignAnimation(RabbitAnims::DIVEFIREQUIT, 5);
				}
			}
			else if (SpeedX) {
				if (runDash > 0)
					runDash = 0;
			}
			else if (lastFire < PlayerProperties.FireSpeed + 30) {
				if (!fireDirection) {
					if (!fixAnim) {
						if (lastFire <= PlayerProperties.FireSpeed + 10)
							AssignAnimation(RabbitAnims::FIRE, PlayerProperties.FireSpeed / 2);
						else
							AssignAnimation(RabbitAnims::QUIT, 5);
					}
				}
				else if (fireDirection == 8) {
					if (!fixAnim) {
						if (lastFire <= PlayerProperties.FireSpeed + 10)
							AssignAnimation(RabbitAnims::FIREUP, PlayerProperties.FireSpeed / 2);
						else
							AssignAnimation(RabbitAnims::FIREUPQUIT, 5);
					}
					++lookVP;
					lastLookVP = gameTicks;
				}
			}
			else if (stonedLen) {
				if (!fixAnim)
					AssignAnimation(RabbitAnims::STONED, 7);
			}
			else if (runDash > 0) {
				if (!fixAnim)
					AssignAnimation(RabbitAnims::STAND, 5);
				if (runDash == 1) {
					if (!fixAnim)
						AssignAnimation(RabbitAnims::REV1, 4, true);
				}
				else if (runDash > 15) {
					if (runDash > 35)
						Particle::AddSpark(HostLevel.Layers[SPRITELAYER], sf::Vector2f(PositionX - 6 * DirectionX, PositionY + 22), -DirectionX);
					fixAnim = 0;
					AssignAnimation(RabbitAnims::REV2, std::max(1, 10 - runDash / 8));
				}
			}
			else if (ledgeWiggle > 0) {
				idleTime = 0;
				if ((++ledgeWiggle <= 5 * GetFrameCount()) || (PlayerProperties.CharacterIndex != char2JAZZ && (ledgeWiggle = 0, true))) { //JJ2+ change; "!= JAZZ" was "== SPAZ"
					if (!fixAnim) {
						if (!UsesAnimation(RabbitAnims::LEDGEWIGGLE)) {
							FrameID = 0;
							frameCount = 0;
							AssignAnimation(RabbitAnims::LEDGEWIGGLE);
						}
						animSpeed = 5;
					}
				}
				else if (!fixAnim) {
					if (!UsesAnimation(RabbitAnims::STAND)) {
						FrameID = 0;
						frameCount = 0;
						AssignAnimation(RabbitAnims::STAND);
					}
					animSpeed = 5;
				}
				if (!frameCount && FrameID == 1 && PlayerProperties.CharacterIndex != char2LORI && UsesAnimation(RabbitAnims::LEDGEWIGGLE)) {
					if (PlayerProperties.CharacterIndex == char2JAZZ)
						PlaySampleAtObject(JazzSounds, BALANCE);
					else
						PlaySampleAtObject(SpazSounds, WOOHOO);
				}
			}
			else if (PlayerProperties.CharacterIndex == char2JAZZ && idleTime > AISPEED * 3) {
				if (AnimID == idleAnim) {
					if (!fixAnim) {
						idleTime = AISPEED;
						AssignAnimation(RabbitAnims::STAND, 8);
					}
				}
				else {
					const int nextidleAnim = RabbitAnims::IDLE1 + RandFac(0x7FFF) % 5;
					idleAnim = RabbitAnimIDs[nextidleAnim];
					AssignAnimation(nextidleAnim, (nextidleAnim == RabbitAnims::IDLE1) ? (8 - (RandFac(3))) : 7, true);
				}

				if (frameCount == 0) {
					if (!UsesAnimation(RabbitAnims::IDLE2)) {
						/*int sampleSample = 0;
						if (UsesAnimation(RabbitAnims::IDLE3)) {
							if (FrameID == 3)
								sampleSample = sJAZZSOUNDS_HEY2;
							else if (FrameID == 11)
								sampleSample = sJAZZSOUNDS_IDLE;
						}
						else if (UsesAnimation(RabbitAnims::IDLE4) && FrameID == 3)
							sampleSample = sJAZZSOUNDS_PFOE;
						if (sampleSample)
							;//PlaySample(PositionX, PositionY, sampleSample, 0, 0);*/ //todo samples
					}
					else {
						/*int sampleSample = 0;
						switch (FrameID) {
						case 7:
							sampleSample = sCOMMON_SWISH6;
							break;
						case 13:
						case 21:
							sampleSample = sCOMMON_EAT1;
							break;
						case 26:
							sampleSample = sCOMMON_EAT3;
							break;
						case 36:
							sampleSample = sCOMMON_IMPACT9;
							break;
						case 31:
							sampleSample = sCOMMON_DOWNFL2;
							break;
						}
						if (sampleSample)
							;//PlaySample(PositionX, PositionY, sampleSample, 0, 0);*/ //todo samples
					}
				}
			}
			else {
				if (PlayerProperties.CharacterIndex == char2SPAZ && idleTime > AISPEED * 2) {
					if (AnimID == idleAnim && !fixAnim) {
						idleTime = 1;
						fixAnim = 0;
						AssignAnimation(RabbitAnims::STAND, 8);
					}
					else if (idleAnim != RabbitAnimIDs[RabbitAnims::IDLE3]) {
						if (AnimID != idleAnim || !fixAnim) {
							idleTime = 0;
							fixAnim = 0;
							switch (RandFac(7)) {
							case 1:
								idleAnim = RabbitAnimIDs[RabbitAnims::IDLE2];
								if (!fixAnim)
									AssignAnimation(RabbitAnims::IDLE2, 8 - RandFac(3), true);
								break;
							case 3:
								idleAnim = RabbitAnimIDs[RabbitAnims::IDLE4];
								if (!fixAnim)
									AssignAnimation(RabbitAnims::IDLE4, 8, true);
								break;
							case 5:
								idleAnim = RabbitAnimIDs[RabbitAnims::IDLE5];
								if (!fixAnim)
									AssignAnimation(RabbitAnims::IDLE5, 8, true);
								/*{
									const int ufoObjectID = AddObject(PositionX, PositionY, aBOUNCEONCE, a1 + PLAYSLOT);
									if (ufoObjectID > 0)
										gameObjects[0][ufoObjectID].behavior = cUFO;
								}*/ //todo ufo
								break;
							case 7:
								if (!idleExtra) {
									idleAnim = RabbitAnimIDs[RabbitAnims::IDLE3];
									break;
								}
							default:
								if (idleAnim != RabbitAnimIDs[RabbitAnims::IDLE1]) {
									idleAnim = RabbitAnimIDs[RabbitAnims::IDLE1];
									if (!fixAnim)
										AssignAnimation(RabbitAnims::IDLE1, 8, true);
								}
								break;
							}
						}
					}
					else {
						DirectionX = 1;
						if (idleTime < AISPEED * 5) {
							/*const int PositionXBird = PositionX + ((140 - idleTime) << 20) / 21 + 0xB40000;
							const int PositionYBird = PositionY - 100 * sintable(idleTime + 116) + 0x100000;
							if (!(gameTicks & 3) && !RandFac(0xF))
								;//PlaySample(PositionXBird, PositionYBird, sSPAZSOUNDS_CHIRP, 0, 0);
							AddSprite(PositionXBird, PositionYBird, SPRITELAYER, 1, 0, 0, anims[AnimBase[AnimSets::Spaz2] + AnimSets::Spaz2_BIRD3].frameList[(gameTicks >> 3) & 7]);
							idleTrail = idleTime;
							if (idleTime >= AISPEED * 4) {
								curAnim = AnimBase[AnimSets::Spaz2] + AnimSets::Spaz2_BIRD1;
								animSpeed = ANIM_SPEED_MAX;
								FrameID = min(3, (idleTime - AISPEED * 4) / 7);
								frameCount = 0;
								fixAnim = 0;
							}
							else if (!fixAnim)*/ //todo spaz bird
								AssignAnimation(RabbitAnims::IDLE1, 8);
						}
						else {
							if (idleTime > 558)
								idleTrail = 0;
							if (!frameCount) {
								/*int sampleSample = 0;
								if (FrameID == 10)
									sampleSample = sSPAZSOUNDS_BIRDSIT;
								else if (FrameID == 15)
									sampleSample = sSPAZSOUNDS_BIRDSIT;
								else if (FrameID == 20)
									sampleSample = sSPAZSOUNDS_BIRDSIT;
								else if (FrameID == 26)
									sampleSample = sSPAZSOUNDS_EATBIRD;
								if (sampleSample)
									;//PlaySample(PositionX, PositionY, sampleSample, 0, 0);*/ //todo samples
							}
							if (!fixAnim) {
								/*if (curAnim == AnimBase[AnimSets::Spaz2] + AnimSets::Spaz2_BIRD2) {
									idleTime = 0;
									idleAnim = 30;
								}
								else {
									curAnim = AnimBase[AnimSets::Spaz2] + AnimSets::Spaz2_BIRD2;
									animSpeed = 8;
									FrameID = 0;
									frameCount = 0;
									fixAnim = 1;
								}*/ //todo spaz bird
							}
						}
					}
				}
				else if ((AnimID == idleAnim) && (frameCount == 0)) {
					if (idleAnim == RabbitAnimIDs[RabbitAnims::IDLE2] && FrameID == 3)
						;//PlaySample(PositionX, PositionY, RandFac(1) ? sSPAZSOUNDS_HAPPY : sSPAZSOUNDS_HOHOHO1, 0, (RandFac(15) << 8) + 11025);
					else if (idleAnim == RabbitAnimIDs[RabbitAnims::IDLE4] && FrameID == 5)
						PlaySampleAtObject(SpazSounds, BURP, 0, 22050);
				}
				else if (PlayerProperties.CharacterIndex == char2LORI && idleTime > AISPEED * 2) {
					idleTime = 0;
					const int pickIdleAnimation = RandFac(7) - 2;
					if (pickIdleAnimation == 0) {
						idleAnim = RabbitAnimIDs[RabbitAnims::IDLE2];
						animSpeed = 8;
					}
					else {
						if (pickIdleAnimation == 1)
							idleAnim = RabbitAnimIDs[RabbitAnims::IDLE3];
						else if (pickIdleAnimation == 2)
							idleAnim = RabbitAnimIDs[RabbitAnims::IDLE4];
						else
							idleAnim = RabbitAnimIDs[RabbitAnims::IDLE1];
						animSpeed = 8 - RandFac(3);
					}
					AnimID = idleAnim;
					FrameID = 0;
					frameCount = 0;
					fixAnim = 1;
				}
				else if (pushObject || (DirectionKeyX > 0 && !goRight) || (DirectionKeyX < 0 && !goLeft)) {
					lastPush = gameTicks;
					++push;
					if (!fixAnim)
						AssignAnimation((push > 16) ? RabbitAnims::PUSH : RabbitAnims::STAND, 8);
				}
				else {
					if (DirectionKeyY < 0) {
						lastLookVP = gameTicks;
						AssignAnimation(RabbitAnims::LOOKUP, ANIM_SPEED_MAX, false, (++lookVP >= 8) ? 1 : 0);
						if (lookVP > 30) {
							if (lookVPAmount > 0)
								lookVPAmount -= 4;
							else if (lookVPAmount > -60)
								lookVPAmount -= 1;
						}
					}
					else if (lastDive < (int)gameTicks && lastDive >(int)(gameTicks - 4))
						AssignAnimation(RabbitAnims::DIVE, ANIM_SPEED_MAX, false);
					else if (!fixAnim)
						AssignAnimation(RabbitAnims::STAND, 8); //the main line that assigns the standing animation
				}
			}
			if (runDash >= -1000) {
				absoluteSpeed = abs(SpeedX);
				if (absoluteSpeed > 0.25f && AccelerationX) {
					if (pushObject) {
						++push;
						lastPush = gameTicks;
					}
					if (!fixAnim) {
						if (push <= 16) {
							if (stonedLen)
								AssignAnimation(RabbitAnims::LOOPY, 6);
							else
								AssignAnimation((absoluteSpeed >= 8 && run) ? RabbitAnims::RUN3 : (absoluteSpeed <= 4) ? RabbitAnims::RUN1 : RabbitAnims::RUN2, 11 - int(absoluteSpeed));
						}
						else
							AssignAnimation(RabbitAnims::PUSH, 8);
					}
				}
			}
			else {
				if (!fixAnim)
					AssignAnimation(RabbitAnims::REV3, 2, true);
				runDash += 1000;
			}
		}
	}
	else if (dive = 0, SpeedY > 0) {
		if (downAttack < 0 || downAttack >= 50) {
			if (helicopter) {
				if (lastFire < PlayerProperties.FireSpeed + 20) {
					if (!fixAnim) {
						if (lastFire <= PlayerProperties.FireSpeed)
							AssignAnimation(RabbitAnims::HELICOPTERFIRERIGHT, PlayerProperties.FireSpeed / 2);
						else
							AssignAnimation(RabbitAnims::HELICOPTERFIREQUIT, 5);
					}
				}
				else if (!fixAnim)
					AssignAnimation(RabbitAnims::HELICOPTER, 4);
			}
			else if (lastFire >= PlayerProperties.FireSpeed + 20) {
				if (!fixAnim)
					AssignAnimation((abs(SpeedX) <= 1) ? RabbitAnims::FALL : RabbitAnims::RIGHTFALL, 8);
			}
			else if (!fixAnim) {
				if (lastFire > PlayerProperties.FireSpeed + 10)
					AssignAnimation(RabbitAnims::JUMPFIREQUIT, 5);
				else
					AssignAnimation(RabbitAnims::JUMPFIRERIGHT, PlayerProperties.FireSpeed / 2);
			}
		}
		else {
			if (downAttack > 4) {
				if (lookVPAmount < 0)
					lookVPAmount += 6;
				else if (lookVPAmount < 20)
					lookVPAmount += 4;
				else if (lookVPAmount < 60)
					lookVPAmount += 2;
				++lookVP;
				lastLookVP = gameTicks;
			}
			if (downAttack >= 40) {
				if (downAttack == 40)
					PlaySampleAtObject(Common, DOWN, 32, 33075);
				if (!fixAnim)
					AssignAnimation(RabbitAnims::FALLBUTTSTOMP, std::min(4, (downAttack - 40) / 8 + 1));
			}
			else if (!fixAnim)
				AssignAnimation(RabbitAnims::SPRING, std::max(1, 5 - downAttack / 16));
		}
	}
	else if (lastFire < PlayerProperties.FireSpeed + 20) {
		if (!fixAnim) {
			if (lastFire > PlayerProperties.FireSpeed + 10)
				AssignAnimation(RabbitAnims::JUMPFIREQUIT, 5);
			else
				AssignAnimation(RabbitAnims::JUMPFIRERIGHT, PlayerProperties.FireSpeed / 2);
		}
	}
	else if ((absoluteSpeed = abs(SpeedX)) < 0.5f) {
		if (!fixAnim) {
			if (specialJump)
				AssignAnimation(RabbitAnims::SPRING, 3);
			else
				AssignAnimation(RabbitAnims::JUMPING3, 8);
		}
	}
	else if (absoluteSpeed <= 8 || !run) {
		if (specialJump) {
			if (!fixAnim)
				AssignAnimation(RabbitAnims::SPRING, 8);
		}
		else {
			if (UsesAnimation(RabbitAnims::RIGHTJUMP)) {
				if (FrameID >= 7)
					AssignAnimation(RabbitAnims::RIGHTJUMP, ANIM_SPEED_MAX, false, 7);
				else if (!fixAnim)
					animSpeed = 1;
			}
			else
				AssignAnimation(RabbitAnims::RIGHTJUMP, ANIM_SPEED_MAX, false);
		}
	}
	else {
		if (!UsesAnimation(RabbitAnims::ROLLING) && PlayerProperties.CharacterIndex == char2SPAZ) {
			const int spazSoundEffect = RandFac(3);
			if (spazSoundEffect <= 1)
				;//PlaySample(PositionX, PositionY, spazSoundEffect ? sSPAZSOUNDS_WOOHOO : sSPAZSOUNDS_OHOH, 0, 0);
		}
		if (!fixAnim)
			AssignAnimation(RabbitAnims::ROLLING, 8);
	}
	if (lastLookVP < gameTicks - 2)
		lookVP = 0;
	if (lastPush < gameTicks - 2)
		push = 0;
	if (lookVP == 0 && lookVPAmount != 0) {
		if (lookVPAmount > 0) {
			if ((lookVPAmount -= 4) < 0)
				lookVPAmount = 0;
		}
		else {
			if ((lookVPAmount += 4) > 0)
				lookVPAmount = 0;
		}
	}
	if (PlayerProperties.CharacterIndex == char2SPAZ && idleTrail > 0 && idleTime < AISPEED * 2) { //part of issue #96
		if (idleExtra == 0)
			idleAnim = RabbitAnimIDs[RabbitAnims::IDLE1];
		if (++idleExtra > AISPEED) {
			idleTrail = 0;
			idleExtra = 1;
		}
	}
}
void Bunny::DepleteAmmo() {
	if (fireType == Weapon::Blaster)//infinite
		return; 
	if (--PlayerProperties.Ammo[fireType] < 1) {
		PlayerProperties.Ammo[fireType] = 0;
		PlayerProperties.Powerups[fireType] = false;
		while (true) {
			--fireType;
			if (fireType == Weapon::Blaster || PlayerProperties.Ammo[fireType] > 0) return;
		}
	}
}
bool Bunny::ProcessActionFire() {
	if (fireType == Weapon::Blaster && shieldTime > 0 && !lastFire)
		return true;
	else {
		const bool hasAmmo = fireType == Weapon::Blaster || PlayerProperties.Ammo[fireType] > 0;
		if ((fireType != Weapon::Toaster
#ifdef BETAPEPPER
			&& fireType != Weapon::Gun8
#endif
			) || lastFire > PlayerProperties.FireSpeed) {
			if (!lastFire && hasAmmo)
				return true;
		} else {
			//if (!wp->bubbles || yPos < GameGlobals->level.waterLevel)
				//loopSample = PlayLoopSample(xPos,yPos,sCOMMON_FLAMER,0,0,loopSample);
			if (hasAmmo) {
				const int adjustedFireSpeed = int(abs(SpeedY / 4));
				if (6 - adjustedFireSpeed <= 1 || !(lastFire % (6 - adjustedFireSpeed)))
					return true;
				else DepleteAmmo();
			}
		}
	}
	return false;
}
bool Bunny::ProcessActionSpecialMove() {
	if (PlayerProperties.CharacterIndex == char2JAZZ) {
		if (goUp) {
			if (specialMove < 16)
				SpeedX = SpeedY = 0;
			else if (specialMove <= 45)
				SpeedY = (specialMove - 256) / 32.f;
			else
				return false;
		} else
			return false;
	} else if (PlayerProperties.CharacterIndex == char2SPAZ) {
		if (DirectionX >= 0) {
			if (!goRight) { SpeedX = SpeedY = 0; return false; }
		} else if (!goLeft) { SpeedX = SpeedY = 0; return false; }
		if (specialMove >= 16) {
			if (specialMove <= 70) {
				SpeedY = (specialMove < 32) ? ((specialMove - 32) / 8.f) : 0;
				SpeedX = float(DirectionX << 4);
			} else return false;
		} else {
			if (specialMove == 2)
				;// PlaySample(xPos, yPos, sSPAZSOUNDS_KARATE7 + RandFac(1), 0, 0);
			SpeedX = 0;
			SpeedY = 0;
		}
	} else { //Lori
		if (specialMove < 16)
			SpeedX = SpeedY = 0;
		else if (specialMove > 45)
			return false;
	}
	return true;
}
void Bunny::ProcessAction(unsigned int gameTicks)
{
	/*if (!!GameGlobals->level.finish && GameGlobals->level.finishCounter > 32768 + 22 * 6) {	//hackish, but I didn't want to rewrite all of AdjustFrogPlayer just for this. ensures that levels always end, no matter your charCurr or curAnim
																							//(22*6 is how long it takes to play a rabbit's end-of-level animation)
		if (NetGlobals->partyMode || *numPlayers != 1 && !IsSinglePlayerBasedGame()) {
			if (!MenuGlobals->menuCode) MenuGlobals->menuCode = quitWAIT;
		}
		else MenuGlobals->menuCode = quitLEVEL;
	}*///todo level ending

	if (freeze) {
		--freeze;
		return;
	}

	/*if (charCurr == mBIRD) //todo non-rabbits
		AdjustBirdPlayer(playerID);
	else if (PlayerProperties.CharacterIndex == char2FROG)
		AdjustFrogPlayer(playerID);
	else*/ {
		if (runDash < 0) {
			++runDash;
			if (DirectionX >= 0) {
				if (!goRight) runDash = 0;
			}
			else if (!goLeft) runDash = 0;
		}
		if (specialMove > 0) {
			++specialMove;
			downAttack = DOWNATTACKLEN;
			if (
				hPole > 0
				|| vPole > 0
				|| sucked
				|| !ProcessActionSpecialMove()
				)
				specialMove = 0;
		}
#ifdef LEDGE_CLIMBING
		oldXPos = xPos;	//outside of ledge climbing, JJ2 never seems to use these properties for local players, so setting them would be harmless but also pointless
		oldYPos = yPos;
#endif
		AdjustRabbit(gameTicks);
		if (fly) { //moved in here to prevent silly farting frog stuff
			if (fly == -1) {
				/*int offset = (versionTSF ? 0 : 1); //todo airboard stuff
				if (airboard <= 0 || airboard >= 14) {
					if (!fixAnim) {
						curAnim = AnimBase[charCurr] + offset;
						animSpeed = 6;
					}
				}
				else {
					curAnim = AnimBase[charCurr] + offset + 1;
					animSpeed = 256;
					FrameID = airboard >> 1;
					frameCount =
						fixAnim =
						fire = 0;
					lastFire = AISPEED;
				}
				if ((gameTicks & 7) == 0)
					AddExplosion(
						xPos - (DirectionX << 20),
						yPos + 8 * sintable(12 * gameTicks) + (12 * FIXMUL),
						AnimBase[mPICKUPS] + 4
					);*/
			}
			/*else if (fly > 1) { //todo copters, etc.
				jjBEHAVIOR& flyingObject = gameObjects[0][fly - 1].behavior;
				if (!(flyingObject == copter) && flyingObject != loadObjects[aCHESHIRE2].behavior) //TODO expand this to AngelScript somehow--metadata?
					fly = 0;
			}*/
		}
		else
			airBoard = 0;
	}

	if (++frameCount <= animSpeed) {
		if (FrameID >= GetFrameCount()) //this should never be true, I think
			FrameID = 0;
	} else {
		const auto numFrames = GetFrameCount();
		frameCount = 0;
		if (++FrameID >= numFrames) {
			FrameID = (fixAnim) ? numFrames - 1 : 0;
			fixAnim = false;
		}
	}

	//if (PlayerProperties.CharacterIndex != char2FROG && PlayerProperties.CharacterIndex != char2BIRDBLUE) //todo non-rabbits
		if (ProcessActionFire())
			AddBullet();
	/*if (charCurr != mFROG)*/ { //todo non-rabbits
		ApproachZero(morph);
	}
	ApproachZero(flicker);
	/*if (isStarted || isOvertime || NetGlobals->partyMode == gameLOCAL)*/ { //todo? game stopping
		if (sugarRush) {
			--sugarRush;
			/*if (--sugarRush <= 0 && SoundGlobals->soundEnabled) //todo music
				ModMusicResume();*/
		}

		if (shieldTime <= -128)
			shieldType = 0;
		else if (--shieldTime > -128)
			{}// ShieldSamples(); //todo samples

		ApproachZero(invincibility);

		/*if (playerTimer[localPlayerID].state == timerSTARTED) { //todo? player timers
			duration = playerTimer[localPlayerID].endTime - gameTicks;
			if (duration <= 0) {
				playerTimer[localPlayerID].state = timerSTOPPED;
				playerTimer[localPlayerID].endTime = playerTimer[localPlayerID].startTime = 0;
				executeASFunction(playerTimer[localPlayerID].endFunction);
			}
			// basically copied from the shield expiring code, though the intervals might be off a bit
			else if (!playerTimer[localPlayerID].soundsDisabled) {
				if (duration < (AISPEED * 3) && !(gameTicks & 15))
					;//PlaySample(xPos, yPos, sCOMMON_BELL_FIRE, 0, 0);
				if (duration < (AISPEED * 5) && !(gameTicks & 7))
					;//PlaySample(xPos, yPos, sCOMMON_BELL_FIRE2, 0, 0);
			}
		}*/
	}
	/*else if (shieldType == 4 && shieldTime >= 40 * AISPEED - 32) //do the laser shield initializing animation thing //todo? shields, game stopping
		--shieldTime;*/
	LowerToZero(stop); //is this ever used?
	//playerTraces[0][(gameTicks & 0x3F) + (playerID << 6)] = (xPos >> 16) + (yPos & 0xFFFF0000); //todo traces
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
	DoZoneDetection(gameState.GetEvent(int(PositionX / TILEWIDTH), int(PositionY / TILEHEIGHT)), gameState.GameTicks);
	ProcessAction(gameState.GameTicks);
	AdjustViewpoint(gameState);

	if (--TraceStartIndex < 0) TraceStartIndex = MAXPLAYERTRACE-1;
	Trace[TraceStartIndex] = sf::Vector2f(PositionX, PositionY);
	if (gameState.GameTicks & 1) {
		if ((run || runDash < 0 || SpeedY >= 10 || spring || sugarRush || specialMove > 10) && !hang) {
			if (TraceLength < MAXPLAYERTRACE)
				TraceLength += 1;
		} else {
			LowerToZero(TraceLength);
		}
	}

	++HelpStringCounter;
}
#include "Windows.h"
void Bunny::Draw(Layer* layers) const {
	if (!flicker || ((long long)(getCurrentTime()) & 64))
		DrawNormally(layers);

	if (TraceLength) {
		const unsigned int summedSpeeds = unsigned int(abs(SpeedX) + abs(SpeedY));
		if (summedSpeeds) //otherwise no point, since this is the minimum intensity
			for (unsigned int j = 0; j / 2 < (TraceLength + 1) / 2; j += 2)
				DrawLightToLightBuffer(LightType::Point, 20, std::min(j * 2, summedSpeeds), Trace[(TraceStartIndex + TraceLength - j) % MAXPLAYERTRACE]);
	}

	if (AmbientLightingLevel < LightingTarget)
		AmbientLightingLevel += 1;
	else if (AmbientLightingLevel > LightingTarget)
		AmbientLightingLevel -= 1;
}

std::array<Player, MAXLOCALPLAYERS> Players;