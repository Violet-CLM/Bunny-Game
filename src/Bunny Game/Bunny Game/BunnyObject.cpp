#include "Bunny.h"
#include "BunnyMisc.h"
#include "BunnyObject.h"
#include "BunnyWeapons.h"
#include "Pickups.h"

void BunnyObject::DrawNormally(Layer* layers, const SpriteMode& mode) const {
	layers[SPRITELAYER].AppendSprite(mode, int(PositionX), int(PositionY), GetFrame(), DirectionX < 0, DirectionY < 0);
}

void BunnyObject::Draw(Layer* layers) const {
	DrawNormally(layers);
}

//helper function for DoBlastBase
static float GetAdjustedBlastSpeed(float d, float limit) {
	d /= 2;
	LimitTo(d, limit);
	return d;
}
#include "windows.h"
#include "Misc.h"
void BunnyObject::DoBlast(int forceRadius, bool doFullBlast) { //used by TNT, RFs, and Bomb
	if (Parent != nullptr) {
		const GameObject* const creator = Parent;
		if (creator->ObjectType == BunnyObjectType::Player && static_cast<const Bunny*>(creator)->IsHurt())
			return;
	}
	
	TNTBullet* nearestTNT = nullptr;
	int nearestTNTDistance = forceRadius;
	for (auto& it : HostLevelObjectList) {
		const bool otherObjectIsPlayer = it->ObjectType == BunnyObjectType::Player;
		bool otherObjectIsPickup = false;
		if (otherObjectIsPlayer || (doFullBlast && (it->ObjectType == BunnyObjectType::Interactive || (otherObjectIsPickup = (it->ObjectType == BunnyObjectType::Pickup))))) {

			const float dx = it->PositionX - PositionX; //swapped order from original code to avoid making GetAdjustedBlastSpeed return -d instead of just d.
			const float dy = it->PositionY - PositionY;
			//if (getPlayerVarSettingLocal(play, pvANTIGRAV))
			//	dy = -dy;

			const int dist = int(dx*dx + dy*dy);
			if (dist < abs(forceRadius)) { //object is close enough to blast away
				if (otherObjectIsPlayer) {
					Bunny& play = static_cast<Bunny&>(*it);
					play.SpeedX = GetAdjustedBlastSpeed(dx, 8);
					play.SpeedY = GetAdjustedBlastSpeed(dy, 12);
					play.beMoved = 30;
					play.idleTime = 0;
					play.spring = 0;
				} else //doFullBlast must be true to have gotten here
				if (otherObjectIsPickup) {
					Pickup& pickup = static_cast<Pickup&>(*it);
					pickup.SpeedX = GetAdjustedBlastSpeed(dx, 6);
					pickup.SpeedY = GetAdjustedBlastSpeed(dy, 6);
				} else { //Interactive
					const int force = 1+((abs(forceRadius)-dist)*8)/abs(forceRadius);
					TNTBullet* itAsTNT = dynamic_cast<TNTBullet*>(it.get());
					if (itAsTNT != nullptr) {
						if (dist < nearestTNTDistance) {
							nearestTNT = itAsTNT;
							nearestTNTDistance = dist;
						}
					} else {
						static_cast<Interactive&>(*it).Hurt(force, true);
					}
				}
			}
		}
	}
	if (nearestTNT != nullptr) //trigger only nearest TNT!
		nearestTNT->Hurt(1, true); //force amount doesn't matter
}

Interactive::Interactive(ObjectStartPos& start, bool enemy) : BunnyObject(start), IsEnemy(enemy), TriggersTNT(enemy) {
	ObjectType = BunnyObjectType::Interactive;
}
void Interactive::Behave(GameState& gameState) {
	Move(gameState);
	LowerToZero(JustHit);
}
void Interactive::Draw(Layer* layers) const {
	DrawNormally(layers, !JustHit ? SpriteMode::Paletted : SpriteMode(Shaders[DefaultShaders::SingleColorPaletted], 15));
}
bool Interactive::Hurt(unsigned int force, bool hurtByBullet) {
	JustHit = 5; //FLASHTIME
	if ((Energy -= force) <= 0) {
		//todo points
		//todo pickups
		//todo particles
		//todo sounds
		Delete();
		return true;
	}
	return false;
}
void Interactive::HitBy(GameObject& other) {
	if (other.ObjectType == BunnyObjectType::Player) {
		Bunny& play = static_cast<Bunny&>(other);
		const auto attackType = play.GetAttackType(false); //false: Interactive is not frozen (todo)
		if (attackType == Bunny::AttackTypes::NotAttacking) {
			if (IsEnemy)
				play.Hurt();
		} else {
			if (attackType != Bunny::AttackTypes::SpecialAttack || CancelSpecialAttacks) //only for bosses
				play.HitEnemyUsingAttackType(attackType);
			Hurt(4, false); //all physical attacks do 4 damage
		}
	} else { //player bullet
		//todo ice
		PlayerBullet& bullet = static_cast<PlayerBullet&>(other);
		bullet.Explode(); //don't need to worry about fireballs I guess
		Hurt(bullet.damage, true);
	}
}
