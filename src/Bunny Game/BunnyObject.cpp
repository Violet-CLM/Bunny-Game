#include "Bunny.h"
#include "BunnyMisc.h"
#include "BunnyObject.h"
#include "BunnyWeapons.h"
#include "BunnyShaders.h"
#include "Pickups.h"

void BunnyObject::DrawNormally(Layer* layers, const SpriteMode& mode) const {
	layers[SPRITELAYER].AppendSprite(mode, int(PositionX), int(PositionY), GetFrame(), DirectionX < 0, DirectionY < 0);
}

void BunnyObject::Draw(Layer* layers) const {
	DrawNormally(layers);
	if (LightType != LightType::None)
		DrawObjectToLightBuffer(*this);
}

void BunnyObject::Behave(GameState &) {
	//todo deactivation?
}

//helper function for DoBlastBase
static float GetAdjustedBlastSpeed(float d, float limit) {
	d /= 2;
	LimitTo(d, limit);
	return d;
}
void BunnyObject::DoBlast(int forceRadius, bool doFullBlast) { //used by TNT, RFs, Seekers (when hitting an object), and Bomb
	if (Parent != nullptr) {
		const GameObject* const creator = Parent;
		if (creator->ObjectType == BunnyObjectType::Player && static_cast<const Bunny*>(creator)->IsHurt())
			return;
	}
	
	TNTBullet* nearestTNT = nullptr;
	int nearestTNTDistance = forceRadius;
	for (auto& it : HostLevel.Objects) {
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

Bunny* BunnyObject::GetNearestPlayer(int threshold) const {
	return GetNearestPlayerRef(threshold);
}
Bunny* BunnyObject::GetNearestPlayerRef(int& threshold) const {
	Bunny* nearest = nullptr;
	for (const auto& it : Players) {
		int totalDistance = int(PositionX - it.Object->PositionX);
		if ((totalDistance *= totalDistance) < threshold) { //necessary, though not sufficient
			const auto dy = int(PositionY - it.Object->PositionY);
			if ((totalDistance += dy * dy) < threshold) {
				threshold = totalDistance;
				nearest = it.Object;
			}
		}
	}
	return nearest;
}

void BunnyObject::PutOnGround(bool walker) {
	if (walker) {
		FrameID = 0;
		PositionY -= 15; //start a bit higher 
	}
	const auto frame = GetFrame();
	const int yStep = walker ? 1 : 4;

	const auto px = int(PositionX) + (((frame.Width / 2) + frame.HotspotX));
	const auto py = walker ? (frame.HotspotY - frame.ColdspotY) : (frame.HotspotY + frame.Height);
	const auto& layer = HostLevel.Layers[SPRITELAYER];
	const auto bottomOfLevel = float(layer.HeightPixels);

	// Move object down until we've hit something
	for (; PositionY < bottomOfLevel; PositionY += yStep)
		if (layer.MaskedPixel(px, int(PositionY) + py)) break;
}


void BunnyObject::MakeRectangularCollisionShapeBasedOnCurrentFrame() {
	CollisionShapes.clear();
	const auto& frame = GetFrame();
	CollisionShapes.emplace_back(frame.Width, frame.Height, float(DirectionX >= 0 ? (frame.HotspotX) : (-frame.Width - frame.HotspotX)), float(frame.HotspotY));
}

Interactive::Interactive(ObjectStartPos& start, bool enemy) : BunnyObject(start), IsEnemy(enemy), TriggersTNT(enemy) {
	ObjectType = BunnyObjectType::Interactive;
}
void Interactive::Behave(GameState& gameState) {
	if (!Frozen) {
		Move(gameState);
		LowerToZero(JustHit);
	} else {
		--Frozen;
	}
}
void Interactive::Draw(Layer* layers) const {
	DrawNormally(layers,
		!JustHit ?
			(Frozen == 0 || (Frozen < 30 && RandFac(1))) ?
				SpriteMode::Paletted :
			SpriteModeFrozen :
		SpriteModeJustHit
	);
}
bool Interactive::Hurt(unsigned int force, bool hurtByBullet) {
	JustHit = 5; //FLASHTIME
	if ((Energy -= force) <= 0) {
		return Die();
	}
	return false;
}
bool Interactive::Die() {
	//todo points
	//todo pickups
	//todo particles
	//todo sounds
	Delete();
	return true;
}
void Interactive::HitBy(GameObject& other) {
	if (other.ObjectType == BunnyObjectType::Player) {
		Bunny& play = static_cast<Bunny&>(other);
		const auto attackType = play.GetAttackType(Frozen);
		if (attackType == Bunny::AttackTypes::NotAttacking) {
			if (IsEnemy && !Frozen)
				play.Hurt();
		} else {
			if (attackType != Bunny::AttackTypes::SpecialAttack || CancelSpecialAttacks) //only for bosses
				play.HitEnemyUsingAttackType(attackType);
			Hurt(4, false); //all physical attacks do 4 damage
		}
	} else { //player bullet
		PlayerBullet& bullet = static_cast<PlayerBullet&>(other);
		bullet.Explode(); //don't need to worry about fireballs I guess
		if (bullet.ammoID == Weapon::Ice)
			Frozen = static_cast<IceBullet&>(other).freeze;
		else
			Hurt(bullet.damage, true);
	}
}
