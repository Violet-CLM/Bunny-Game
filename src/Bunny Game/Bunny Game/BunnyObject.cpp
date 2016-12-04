#include "Bunny.h"
#include "BunnyMisc.h"
#include "BunnyObject.h"
#include "BunnyWeapons.h"

void BunnyObject::DrawNormally(Layer* layers, const SpriteMode& mode) const {
	layers[SPRITELAYER].AppendSprite(mode, int(PositionX), int(PositionY), GetFrame(), DirectionX < 0, DirectionY < 0);
}

void BunnyObject::Draw(Layer* layers) const {
	DrawNormally(layers);
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
