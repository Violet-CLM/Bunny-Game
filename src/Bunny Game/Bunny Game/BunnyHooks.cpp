#include "Lattice.h"
#include "BunnyVersionDependentStuff.h"
#include "BunnyObjectList.h"
#include "BunnyObject.h"
#include "BunnyShaders.h"
#include "Windows.h"
#include "Misc.h"

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

void Hook_LevelLoad(Level& level, PreloadedAnimationsList& animList) {
	animList = DefaultAnimationsList;

	for (unsigned int layerID = 0; layerID < LEVEL_LAYERCOUNT; ++layerID) { //generate texture from layer 8's tilemap for use in textured background/s
		if (level.Layers[layerID].Get_IsTextured()) { //at least one layer is supposed to be drawn as textured
			Layer& backgroundLayer = level.Layers[LEVEL_LAYERCOUNT-1];
			sf::RenderTexture textureImage;
			if (textureImage.create(8*TILEWIDTH, 8*TILEHEIGHT)) {
				backgroundLayer.MakeTexture(textureImage);
				backgroundLayer.AdditionalTextures.emplace_back(textureImage.getTexture());
				sf::Texture& texture = backgroundLayer.AdditionalTextures.back();
				texture.setRepeated(true);
				texture.setSmooth(true);
				Shaders[BunnyShaders::WarpHorizon]->setUniform("texture256", texture);
				//Shaders[BunnyShaders::WarpHorizon]->setUniform("fadeColor", sf::Glsl::Vec4(sf::Color::Magenta));
				WarpHorizonRenderStates.shader = Shaders[BunnyShaders::WarpHorizon];
			} else { //I have no idea why this might happen but from time to time I decide to write some error-checking
				ShowErrorMessage(L"Failed to create textured background texture!");
			}
			break;
		}
	}
}

bool Hook_Init() {
	if (!IsTSF(VersionTSF))
		return false;
	InitializeRabbitAnimIDs();

	GetDefaultAnimList(DefaultAnimationsList);

	WriteBunnyShaders(); //fill in BunnyShaderSources
	ShaderSources.insert(ShaderSources.end(), BunnyShaderSources.begin(), BunnyShaderSources.end());
	PaletteLineCount = BunnyPaletteLineNames::LAST;

	return true;
}