#include "Lattice.h"
#include "BunnyVersionDependentStuff.h"
#include "BunnyObjectList.h"
#include "BunnyObject.h"
#include "BunnyShaders.h"

static PreloadedAnimationsList DefaultAnimationsList;

bool Hook_CollideObjects(const GameObject& a, const GameObject& b) {
	if ((a.ObjectType == BunnyObjectType::Player || a.ObjectType == BunnyObjectType::PlayerBullet) && (b.ObjectType == BunnyObjectType::Pickup || b.ObjectType == BunnyObjectType::Interactive))
		return true;
	if (a.ObjectType == BunnyObjectType::Player && b.ObjectType == BunnyObjectType::EnemyBullet)
		return true;
	//todo
	return false;
}

void Hook_ActivateObjects(Level& level) {
	level.ForEachEvent([&level](Event& ev, int xTile, int yTile) {
		if (!ev.Active && !ev.Difficulty /* ev.ID >= EventIDs::GUN3AMMO3*/ && Lattice::ObjectInitializationList->count(ev.ID) && Lattice::ObjectInitializationList->at(ev.ID).CreateObjectFromEventMap) { //todo better difficulty check
			Lattice::ObjectInitializationList->at(ev.ID).AddObject(level, ev, float(xTile * TILEWIDTH + (TILEWIDTH/2)), float(yTile * TILEHEIGHT + (TILEHEIGHT/2)));
			ev.Active = true;
		}
	});
}

void Hook_LevelLoad(Level&, PreloadedAnimationsList& animList) {
	animList = DefaultAnimationsList;
}

bool Hook_Init() {
	if (!IsTSF(VersionTSF))
		return false;
	InitializeRabbitAnimIDs();

	GetDefaultAnimList(DefaultAnimationsList);

	ShaderSources.insert(ShaderSources.end(), BunnyShaderSources.begin(), BunnyShaderSources.end());
	PaletteLineCount = BunnyPaletteLineNames::LAST;

	return true;
}