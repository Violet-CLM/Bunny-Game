#include "Lattice.h"
#include "BunnyVersionDependentStuff.h"
#include "BunnyObjectList.h"
#include "BunnyObject.h"
#include "BunnyShaders.h"
#include "BunnyMenu.h"
#include "Bunny.h"
#include "PostProcessing.h"
#include "AdditionalSprites.h"
#include "J2S.h"
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

class ObjectActivationOperation {
	Level& level;
public:
	ObjectActivationOperation(Level& l) : level(l) { }

	void operator()(Event& ev, int xTile, int yTile) {
		if (!ev.Active && Lattice::ObjectInitializationList->count(ev.ID) && Lattice::ObjectInitializationList->at(ev.ID).CreateObjectFromEventMap) {
			Lattice::ObjectInitializationList->at(ev.ID).AddObject(level, ev, float(xTile * TILEWIDTH + (TILEWIDTH/2)), float(yTile * TILEHEIGHT + (TILEHEIGHT/2)));
			ev.Active = true;
		}
	}
};
void Hook_ActivateObjects(Level& level) {
	const ObjectActivationOperation activationFunction(level);

	for (auto& it : Players) {
		Bunny* play = it.Object;
		const int eventActivationXTile = int(play->PositionX) >> 5;
		const int xDiff = eventActivationXTile - play->LastEventActivationXTile;
		if (xDiff) {
			level.ForEachEvent(
				activationFunction,
				sf::Rect<int>(
					play->LastEventActivationXTile + (xDiff > 0 ? 20 : -20),
					play->LastEventActivationYTile - 20,
					abs(xDiff),
					(20 * 2) + 1
				)
			);
			play->LastEventActivationXTile = eventActivationXTile;
		}
		const int eventActivationYTile = int(play->PositionY) >> 5;
		const int yDiff = eventActivationYTile - play->LastEventActivationYTile;
		if (yDiff) {
			level.ForEachEvent(
				activationFunction,
				sf::Rect<int>(
					play->LastEventActivationXTile - 20,
					play->LastEventActivationYTile + (yDiff > 0 ? 20 : -20),
					(20 * 2) + 1,
					abs(yDiff)
				)
			);
			play->LastEventActivationYTile = eventActivationYTile;
		}
	}


	for (auto& it : level.Objects)
		if (it->Active)
			static_cast<BunnyObject*>(it.get())->CheckActiveStatus();
}

void Hook_GetAnimationList(Level&, PreloadedAnimationsList& animList) {
	animList = DefaultAnimationsList;
}
void Hook_LevelLoad(Level& level) {
	const ObjectActivationOperation activationFunction(level);
	for (auto& player : Players) {
		EventID soughtEventID = EventIDs::JAZZSTART;
		std::vector<sf::Vector2i> StartPositions;
		auto FindStartPositions = [&level, &StartPositions, &soughtEventID](Event& ev,int xTile,int yTile) -> void {
			if (ev.ID == soughtEventID)
				StartPositions.emplace_back(xTile, yTile);
		};
		level.ForEachEvent(FindStartPositions);
		if (StartPositions.empty()) {
			soughtEventID = EventIDs::SPAZSTART; //not quite how this should work obviously but it'll do for now... todo
			level.ForEachEvent(FindStartPositions);
			if (StartPositions.empty()) {
				soughtEventID = EventIDs::MPSTART;
				level.ForEachEvent(FindStartPositions);
				if (StartPositions.empty())
					ShowErrorMessage(L"No start positions found!");
			}
		}
		const auto bunnyStartPosition = StartPositions[0]; //should probably be randomly chosen, not the first one... todo
		const static int AnimSetsAssociatedWithEachRabbit[] = {AnimSets::Jazz, AnimSets::Spaz, AnimSets::Lori};
		const int characterID = RandFac(255) % (2 + VersionTSF);
		level.Objects.emplace_front( //wow, this is really wordy, needs more shortcuts
			new Bunny(
				ObjectStartPos(
					level,
					level.GetEvent(
						bunnyStartPosition.x,
						bunnyStartPosition.y)
					,
					float(bunnyStartPosition.x * TILEWIDTH + 15),
					float(bunnyStartPosition.y * TILEHEIGHT + 15),
					GetVersionSpecificAnimationID(AnimSetsAssociatedWithEachRabbit[characterID])
				),
				characterID,
				player
			)
		);
		level.ForEachEvent(
			activationFunction, //activate all object events in a 41x41 square around the player's starting position
			sf::Rect<int>(
				bunnyStartPosition.x - 20,
				bunnyStartPosition.y - 20,
				(20 * 2) + 1,
				(20 * 2) + 1
			)
		);
	}

	//generate texture from layer 8's tilemap for use in textured background/s
	for (unsigned int layerID = 0; layerID < LEVEL_LAYERCOUNT; ++layerID) {
		if (level.Layers[layerID].Get_IsTextured()) { //at least one layer is supposed to be drawn as textured
			Layer& backgroundLayer = level.Layers[LEVEL_LAYERCOUNT-1];
			static sf::RenderTexture textureImage; //Some graphics cards (e.g. mine) crash when a RenderTexture goes out of scope, so this has to be static and carefully reused across levels
			if (textureImage.getSize().x || textureImage.create(8*TILEWIDTH, 8*TILEHEIGHT)) { //if getSize().x is non-zero, then sf::RenderTexture::create has already been called
				backgroundLayer.MakeTexture(textureImage);
				backgroundLayer.AdditionalTextures.emplace_back(textureImage.getTexture());
				sf::Texture& texture = backgroundLayer.AdditionalTextures.back();
				texture.setRepeated(true);
				//texture.setSmooth(true); //doesn't look good on all textures, unfortunately
				Shaders[BunnyShaders::WarpHorizon]->setUniform("texture256", texture);
				Shaders[BunnyShaders::WarpHorizon]->setUniform("fadeColor", sf::Glsl::Vec4(backgroundLayer.FadeColor));
				Shaders[BunnyShaders::Tunnel]->setUniform("texture256", texture);
				Shaders[BunnyShaders::Tunnel]->setUniform("fadeColor", sf::Glsl::Vec4(backgroundLayer.FadeColor));
				Shaders[BunnyShaders::Tunnel]->setUniform("spiral", backgroundLayer.ParallaxStars ? 0.25f : 0.0f);
			} else { //I have no idea why this might happen but from time to time I decide to write some error-checking
				ShowErrorMessage(L"Failed to create textured background texture!");
			}
			break;
		}
	}

	//difficulty
	level.ForEachEvent([&level](Event& ev, int xTile, int yTile) {
		switch (ev.Difficulty) {
		case 1: //Easy
			if (GameDifficulty != Difficulty::Easy)
				ev = 0;
			break;
		case 2: //Hard
			if (GameDifficulty < Difficulty::Hard)
				ev = 0;
			break;
		case 3: //MP-only
			if (Players.size() <= 1)
				ev = 0;
			break;
		}
	});

	//lighting
	ClearLightingBuffer(AmbientLightingLevel = level.MinLight);
	CurrentStageType = StageType::Level;
}

bool Hook_Init() {
	if (!IsTSF(VersionTSF))
		return false;
	InitializeRabbitAnimIDs();
	InitLoadStrings();

	GetDefaultAnimList(DefaultAnimationsList);

	WriteBunnyShaders(); //fill in BunnyShaderSources
	ShaderSources.insert(ShaderSources.end(), BunnyShaderSources.begin(), BunnyShaderSources.end());
	PaletteLineCount = BunnyPaletteLineNames::LAST;

	return true;
}
void Hook_InitAfterShadersConstructed() {
	WarpHorizonRenderStates.shader = Shaders[BunnyShaders::WarpHorizon];
	TunnelRenderStates.shader = Shaders[BunnyShaders::Tunnel];
	MenuBGRenderStates.shader = Shaders[BunnyShaders::MenuBG];
	InitLighting();
	InitCreateAdditionalSprites();
}
void Hook_DetermineInitialStage(std::stack<std::unique_ptr<Stage>>& stages, int argc, char *argv[]) {
	std::wstring filename;
	if (argc == 2)
		filename = WStringFromCharArray(argv[1]);
	if (!filename.empty()) {
		Level* CurrentLevel = Level::LoadLevel(filename);
		if (CurrentLevel != nullptr)
			stages.emplace(CurrentLevel);
	} else
		stages.emplace(new BunnyMenu());
}
