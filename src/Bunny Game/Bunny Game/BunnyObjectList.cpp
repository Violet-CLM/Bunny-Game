#include <memory>
#include "Level.h"
#include "BunnyObjectList.h"
#include "Pickups.h"
#include "Bunny.h"
#include "Diamondus.h"
#include "BunnyVersionDependentStuff.h"

static ObjectList ObjectInitializationList;
#define Obj(a, b, c, d, ...) {EventIDs::a, {GetVersionSpecificAnimationID(AnimSets::b, isTSF), [](ObjectStartPos& objStart){ return (GameObject*)(new c(__VA_ARGS__)); }, d}}
#define ObjT(a, b, c) Obj(a, b, c, true, objStart)
#define ObjTC(a, b, c, ...) Obj(a, b, c, true, objStart, __VA_ARGS__)
ObjectList* GetObjectList(bool isTSF) {
	return &(ObjectInitializationList = {
		ObjT(JAZZSTART, Jazz, Bunny),//todo

		ObjTC(GUN2AMMO3, Ammo, AmmoPickup, 1, 25, 24),
		ObjTC(GUN3AMMO3, Ammo, AmmoPickup, 2, 29, 28),
		ObjTC(GUN4AMMO3, Ammo, AmmoPickup, 3, 34, 33),
		ObjTC(GUN5AMMO3, Ammo, AmmoPickup, 4, 49, 48),
		ObjTC(GUN6AMMO3, Ammo, AmmoPickup, 5, 57, 56),
		ObjTC(GUN7AMMO3, Ammo, AmmoPickup, 6, 59, 59),
		ObjTC(GUN8AMMO3, Ammo, AmmoPickup, 7, 62, 61),
		ObjTC(GUN9AMMO3, Ammo, AmmoPickup, 8, 68, 67),

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
		ObjTC(COKE, Pickups, Food, 20),
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
		ObjTC(MILK, Pickups, Food, 53),
		ObjTC(ORANGE, Pickups, Food, 71),
		ObjTC(PEACH, Pickups, Food, 73),
		ObjTC(PEAR, Pickups, Food, 74),
		ObjTC(PEPSI, Pickups, Food, 75),
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
	});
}

PreloadedAnimationsList GetDefaultAnimList(bool isTSF) {
	PreloadedAnimationsList retval = {
		GetVersionSpecificAnimationID(AnimSets::Ammo, isTSF),
		GetVersionSpecificAnimationID(AnimSets::Bird, isTSF),
		GetVersionSpecificAnimationID(AnimSets::Common, isTSF),
		GetVersionSpecificAnimationID(AnimSets::Faces, isTSF),
		GetVersionSpecificAnimationID(AnimSets::Font, isTSF),
		GetVersionSpecificAnimationID(AnimSets::Jazz, isTSF),
		GetVersionSpecificAnimationID(AnimSets::JazzSounds, isTSF),
		GetVersionSpecificAnimationID(AnimSets::MenuSounds, isTSF),
		GetVersionSpecificAnimationID(AnimSets::Pickups, isTSF),
		GetVersionSpecificAnimationID(AnimSets::Rush, isTSF),
		GetVersionSpecificAnimationID(AnimSets::Spaz, isTSF),
		GetVersionSpecificAnimationID(AnimSets::Spaz2, isTSF),
		GetVersionSpecificAnimationID(AnimSets::SpazSounds, isTSF),
		GetVersionSpecificAnimationID(AnimSets::Spring, isTSF)
	};
	if (isTSF)
		retval.insert({
			GetVersionSpecificAnimationID(AnimSets::Lori, isTSF),
			GetVersionSpecificAnimationID(AnimSets::Lori2, isTSF),
			GetVersionSpecificAnimationID(AnimSets::LoriSounds, isTSF)
		});
	return retval;
}

bool ObjectsShouldCollide(const GameObject& a, const GameObject& b) {
	return false; //todo
}

void ShouldObjectsBeActive(Level& level) {
	level.ForEachEvent([&level](Event& ev, int xTile, int yTile) {
		if (!ev.Active && !ev.Difficulty && ObjectInitializationList.count(ev.ID) && ObjectInitializationList[ev.ID].CreateObjectFromEventMap) { //todo better difficulty check
			ObjectInitializationList[ev.ID].AddObject(level, xTile * TILEWIDTH + (TILEWIDTH/2), yTile * TILEHEIGHT + (TILEHEIGHT/2));
			ev.Active = true;
		}
	});
}