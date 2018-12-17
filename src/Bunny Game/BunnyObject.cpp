#include "Bunny.h"
#include "BunnyMisc.h"
#include "BunnyObject.h"
#include "BunnyWeapons.h"
#include "BunnyShaders.h"
#include "BunnyObjectList.h"
#include "BunnyVersionDependentStuff.h"
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
						static_cast<Interactive&>(*it).Hurt(force, dynamic_cast<Bunny*>(Parent), ParticleExplosionType::Bullet);
					}
				}
			}
		}
	}
	if (nearestTNT != nullptr) //trigger only nearest TNT!
		nearestTNT->Hurt(1, nullptr, ParticleExplosionType::Bullet); //force amount doesn't matter
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

bool BunnyObject::GivePoints(Bunny& play, unsigned int& points) {
	if (points) {
		Particle::AddScore(
			HostLevel.Layers[SPRITELAYER],
			sf::Vector2f(PositionX, PositionY),
			int(points),
			&HostLevel.GetAnimSet(GetVersionSpecificAnimationID(AnimSets::Font)).Animations[1]
		);
		play.PlayerProperties.Score += points;
		points = 0;
		return true;
	}
	return false;
}

Interactive::Interactive(ObjectStartPos& start, unsigned int points, bool enemy) : BunnyObject(start), Points(points), IsEnemy(enemy), TriggersTNT(enemy) {
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
	if (Frozen)
		DrawLightToLightBuffer(LightType::Normal, 56, 80, sf::Vector2f(PositionX, PositionY));

}
bool Interactive::Hurt(unsigned int force, Bunny* play, ParticleExplosionType causeOfDeath) {
	JustHit = 5; //FLASHTIME
	if ((Energy -= force) <= 0) {
		return Die(play, causeOfDeath);
	}
	return false;
}
void Interactive::Explode(ParticleExplosionType causeOfDeath) {
	const auto& frame = GetFrame();
	const auto left = PositionX + frame.HotspotX * DirectionX;
	const auto top = PositionY + frame.HotspotY * DirectionY;
	auto imageWithPaddingAtBottom(frame.Image8Bit);
	imageWithPaddingAtBottom.resize(imageWithPaddingAtBottom.size() + frame.Width * 2 + 2); //some rows of transparent pixels
	unsigned int maximumNumberOfParticlesToGenerate = (frame.Width * frame.Height) >> 4;
	auto& layer = HostLevel.Layers[SPRITELAYER];
	unsigned int numberOfUnsuccessfulAttemptsToFindAPixel = 0;
	do {
		const auto locationInImage = rand() % frame.Image8Bit.size();
		sf::Uint8& pixel = imageWithPaddingAtBottom.at(locationInImage);
		if (pixel) {
			numberOfUnsuccessfulAttemptsToFindAPixel = 0;
			const auto location = sf::Vector2f(
				left + int(locationInImage % frame.Width) * DirectionX,
				top + int(locationInImage / frame.Width) * DirectionY
			);
			if (causeOfDeath == ParticleExplosionType::Bullet || causeOfDeath == ParticleExplosionType::PhysicalAttack) {
				auto particle = Particle::AddPixel(layer, location);
				if (particle) {
					if (causeOfDeath == ParticleExplosionType::PhysicalAttack) { //more extreme
						particle->SpeedX = (8 * int(RandFac(0x7FFF)) - 131064) / 65536.f;
						particle->SpeedY = 8 * ((-16384 - int(RandFac(0x7FFF)))) / 65536.f;
					} else { //less extreme
						particle->SpeedX = (4 * int(RandFac(0x7FFF)) - 65532) / 65536.f;
						particle->SpeedY = 4 * ((-16384 - int(RandFac(0x7FFF)))) / 65536.f;
					}

					auto* target = particle->Pixel.color;
					for (int x = 0; x < particle->size; ++x)
						for (int y = 0; y < particle->size; ++y)
							*target++ = imageWithPaddingAtBottom[locationInImage + x + y*frame.Width];

				} else
					break;
			} else {
				auto particle = Particle::AddFire(layer, location, causeOfDeath == ParticleExplosionType::OrangeShards ? 40 : causeOfDeath == ParticleExplosionType::BlueShards ? 32 : 72); //orange, blue, gray
				if (particle) {
					particle->SpeedY = RandFac(0x7FFF) / 65536.f;
					particle->size = RandFac(3) + 1;
				} else
					break;
			}
			pixel = 0; //don't sample this one again
		} else if (++numberOfUnsuccessfulAttemptsToFindAPixel >= 16)
			break;
	} while (--maximumNumberOfParticlesToGenerate);
}
bool Interactive::Die(Bunny* play, ParticleExplosionType causeOfDeath) {
	if (play != nullptr)
		GivePoints(*play, Points);
	if (!Frozen)
		Explode(causeOfDeath);
	//else
		//todo unfreeze
	//todo sounds
	Delete();
	return true;
}
void Interactive::HitBy(GameObject& other) {
	if (other.ObjectType == BunnyObjectType::Player) {
		Bunny& play = static_cast<Bunny&>(other);
		const auto attackType = play.GetAttackType(!!Frozen);
		if (attackType == Bunny::AttackTypes::NotAttacking) {
			if (IsEnemy && !Frozen)
				play.Hurt();
		} else {
			if (attackType != Bunny::AttackTypes::SpecialAttack || CancelSpecialAttacks) //only for bosses
				play.HitEnemyUsingAttackType(attackType);
			Hurt(4, &play, ParticleExplosionType::PhysicalAttack); //all physical attacks do 4 damage
		}
	} else { //player bullet
		PlayerBullet& bullet = static_cast<PlayerBullet&>(other);
		bullet.Explode(); //don't need to worry about fireballs I guess
		if (bullet.ammoID == Weapon::Ice)
			Frozen = static_cast<IceBullet&>(other).freeze;
		else
			Hurt(bullet.damage, dynamic_cast<Bunny*>(bullet.Parent), bullet.ParticleExplosionType);
	}
}
