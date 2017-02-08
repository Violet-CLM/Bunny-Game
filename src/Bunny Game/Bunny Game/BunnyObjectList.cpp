#include <memory>
#include "Level.h"
#include "BunnyObjectList.h"
#include "Pickups.h"
#include "BunnyEffects.h"
#include "Bunny.h"
#include "Diamondus.h"
#include "BunnyVersionDependentStuff.h"

static ObjectList ObjectInitializationList;

#define Obj(a, b, c, ...) {EventIDs::a, {GetVersionSpecificAnimationID(AnimSets::b), [](ObjectStartPos& objStart){ return (GameObject*)(new c(__VA_ARGS__)); }, EventIDs::a >= 33}}
#define ObjT(a, b, c) Obj(a, b, c, objStart)
#define ObjTC(a, b, c, ...) Obj(a, b, c, objStart, __VA_ARGS__)
const ObjectList& Hook_GetObjectList() {
	Pickup::ExplosionSetID = GetVersionSpecificAnimationID(AnimSets::Pickups);
	return ObjectInitializationList = {
		ObjT(EXPLOSION, Ammo, Explosion),

		ObjTC(GUN2AMMO3, Ammo, AmmoPickup, Weapon::Bouncer),
		//ObjTC(GUN3AMMO3, Ammo, AmmoPickup, Weapon::Ice),
		ObjTC(GUN4AMMO3, Ammo, AmmoPickup, Weapon::Seeker),
		ObjTC(GUN5AMMO3, Ammo, AmmoPickup, Weapon::RF),
		ObjTC(GUN6AMMO3, Ammo, AmmoPickup, Weapon::Toaster),
		ObjTC(GUN7AMMO3, Ammo, AmmoPickup, Weapon::TNT),
		ObjTC(GUN8AMMO3, Ammo, AmmoPickup, Weapon::Gun8),
		//ObjTC(GUN9AMMO3, Ammo, AmmoPickup, Weapon::Gun9),

		ObjTC(PLAYERBULLET1, Ammo, BlasterBullet, false),
		ObjTC(PLAYERBULLETP1, Ammo, BlasterBullet, true),
		ObjT(PLAYERBULLET2, Ammo, BouncerBullet),
		ObjT(PLAYERBULLETP2, Ammo, BouncerBulletPU),
		ObjTC(PLAYERBULLET4, Ammo, SeekerBullet, false),
		ObjTC(PLAYERBULLETP4, Ammo, SeekerBullet, true),
		ObjTC(PLAYERBULLET5, Ammo, RFBullet, false),
		ObjTC(PLAYERBULLETP5, Ammo, RFBullet, true),
		ObjTC(PLAYERBULLET6, Ammo, ToasterBullet, false),
		ObjTC(PLAYERBULLETP6, Ammo, ToasterBullet, true),
		ObjT(TNT, Ammo, TNTBullet),
		ObjTC(PLAYERBULLET8, Ammo, PepperSprayBullet, false),
		ObjTC(PLAYERBULLETP8, Ammo, PepperSprayBullet, true),

		ObjTC(APPLE, Pickups, Food, 1),
		ObjTC(BANANA, Pickups, Food, 2),
		ObjTC(BURGER, Pickups, Food, 11),
		ObjTC(CAKE, Pickups, Food, 12),
		ObjTC(CANDY, Pickups, Food, 13),
		ObjTC(CHEESE, Pickups, Food, 15),
		ObjTC(CHERRY, Pickups, Food, 16),
		ObjTC(CHICKLEG, Pickups, Food, 17),
		ObjTC(CHIPS, Pickups, Food, 18),
		ObjTC(CHOCBAR, Pickups, Food, 19),
		ObjTC(COKE, Pickups, Food, 20, true),
		ObjTC(CUCUMB, Pickups, Food, 23),
		ObjTC(CUPCAKE, Pickups, Food, 24),
		ObjTC(DONUT, Pickups, Food, 25),
		ObjTC(EGGPLANT, Pickups, Food, 26),
		ObjTC(FRIES, Pickups, Food, 32),
		ObjTC(GRAPES, Pickups, Food, 38),
		ObjTC(HAM, Pickups, Food, 39),
		ObjTC(ICECREAM, Pickups, Food, 43),
		ObjTC(LEMON, Pickups, Food, 48),
		ObjTC(LETTUCE, Pickups, Food, 49),
		ObjTC(LIME, Pickups, Food, 50),
		ObjTC(MILK, Pickups, Food, 53, true),
		ObjTC(ORANGE, Pickups, Food, 71),
		ObjTC(PEACH, Pickups, Food, 73),
		ObjTC(PEAR, Pickups, Food, 74),
		ObjTC(PEPSI, Pickups, Food, 75, true),
		ObjTC(PIE, Pickups, Food, 76),
		ObjTC(PIZZA, Pickups, Food, 77),
		ObjTC(PRETZEL, Pickups, Food, 79),
		ObjTC(SANDWICH, Pickups, Food, 80),
		ObjTC(STRAWBERRY, Pickups, Food, 81),
		ObjTC(TACOBELL, Pickups, Food, 88),
		ObjTC(THING, Pickups, Food, 89),
		ObjTC(WEENIE, Pickups, Food, 91),
		ObjTC(WMELON, Pickups, Food, 92),

		ObjT(EXTRALIFE, Pickups, ExtraLife),
		ObjT(ENERGYUP, Pickups, Carrot),
		ObjTC(REDGEM, Pickups, Gem, 0),
		ObjTC(GREENGEM, Pickups, Gem, 1),
		ObjTC(BLUEGEM, Pickups, Gem, 2),
		ObjTC(PURPLEGEM, Pickups, Gem, 3),
		ObjTC(RECTREDGEM, Pickups, Gem, 0),
		ObjTC(RECTGREENGEM, Pickups, Gem, 1),
		ObjTC(RECTBLUEGEM, Pickups, Gem, 2),
		ObjT(GUNFASTFIRE, Pickups, FastFire),
		ObjTC(FASTFEET, Pickups, Pickup, 33),
		ObjT(GOLDCOIN, Pickups, GoldCoin),
		ObjT(FLYCARROT, Pickups, FlyCarrot),
		ObjT(FREEZER, Pickups, FreezeEnemies),
		ObjT(INVINCIBILITY, Pickups, Invincibility),
		ObjT(FULLENERGY, Pickups, FullEnergy),
		ObjT(SILVERCOIN, Pickups, SilverCoin),
		ObjT(STOPWATCH, Pickups, StopWatch),
		ObjTC(EXTRATIME, Pickups, Pickup, 87),

		ObjT(BUMBEE, BumBee, Bee),
	};
}

void GetDefaultAnimList(PreloadedAnimationsList& animList) {
	std::vector<int> retval = {AnimSets::Ammo, AnimSets::Bird, AnimSets::Common, AnimSets::Faces, AnimSets::Font, AnimSets::Jazz, AnimSets::JazzSounds, AnimSets::MenuSounds, AnimSets::Pickups, AnimSets::Rush, AnimSets::Spaz, AnimSets::Spaz2, AnimSets::SpazSounds, AnimSets::Spring};
	if (VersionTSF) {
		const static std::array<int, 3> TSFAnimSetIDs = {AnimSets::Lori, AnimSets::Lori2, AnimSets::LoriSounds};
		retval.insert(retval.end(), TSFAnimSetIDs.begin(), TSFAnimSetIDs.end());
	}
	std::transform(retval.begin(), retval.end(), retval.begin(),  [](int a) { return GetVersionSpecificAnimationID(a); });
	animList = PreloadedAnimationsList(retval.begin(), retval.end());
}