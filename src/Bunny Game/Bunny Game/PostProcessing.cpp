#include "PostProcessing.h"
#include "Bunny.h"
#include "BunnyObject.h"
#include "BunnyObjectList.h"
#include "BunnyWeapons.h"
#include "BunnyMisc.h"
#include "BunnyShaders.h"
#include "BunnyVersionDependentStuff.h"
#include "Resources.h"
#include "CharStrings.h"
#include "Lattice.h"

typedef unsigned int LightHash;
typedef void GenerateLightingSprite(sf::Uint32*, LightParam, LightParam);

float AmbientLightingLevel = 0.25f;

sf::Transform Layer4Offset;
SpriteManager EffectSprites;
sf::RenderTexture LightingBuffer;
sf::RenderStates LightingStates;
VertexCollectionQueue LightingSprites, HUD;
SpriteMode *LightModeAdd, *LightModeAlpha;//, *LightModeMax;
std::unordered_map<LightHash, AnimFrame> LightingSpriteProperties;

void InitLighting() {
	LightingBuffer.create(WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS);
	LightingStates = sf::RenderStates(sf::BlendNone, sf::Transform::Identity, nullptr, Shaders[BunnyShaders::AmbientLighting]);
	Shaders[BunnyShaders::AmbientLighting]->setUniform("dark", sf::Glsl::Vec4());
	Shaders[BunnyShaders::AmbientLighting]->setUniform("lightBuffer", LightingBuffer.getTexture());
	LightModeAdd = new SpriteMode(Shaders[DefaultShaders::Normal], 0, sf::BlendAdd);
	LightModeAlpha = new SpriteMode(Shaders[DefaultShaders::Normal], 0, sf::BlendAlpha);
}

void Hook_ClearSpriteQueues() {
	HUD.Clear();
	LightingSprites.Clear();
}
void Hook_LevelMain(Level& level, unsigned int GameTicks) {
	const AnimSet& fontSet = level.GetAnimSet(GetVersionSpecificAnimationID(AnimSets::Font));
	const struct hm_t { //using resolution width >= 400 values
		const std::vector<AnimFrame>& largerFont;
		const std::vector<AnimFrame>& smallerFont;
		unsigned int lineHeightShift = 1;
		unsigned int lineHeight = 10;
		hm_t(const AnimSet& set) : largerFont(*set.Animations[2].AnimFrames), smallerFont(*set.Animations[0].AnimFrames) {}
	} hm(fontSet); //recreating this struct makes it easier to paste in JJ2+ code
	const std::vector<AnimFrame>& smallFont = *fontSet.Animations[1].AnimFrames;
	
	char buffer[16];

	const auto& play = Players[0]; //no splitscreen support at present
	//for (const auto& play : Players) if (play.Object != nullptr) {
		const auto& playerObject = *play.Object;

		{ //score
			/*
			int scoreDiff = play->score - play->lastScoreDisplay;
			if (scoreDiff > 50) {
				play->lastScoreDisplay += 50;
			} else if (scoreDiff > 10) {
				play->lastScoreDisplay += 10;
			} else if (scoreDiff > 2) {
				play->lastScoreDisplay += 2;
			} else {
				play->lastScoreDisplay = play->score;
			}
			*/
			sprintf_s(buffer, "%07d", playerObject.PlayerProperties.Score);
			WriteText(HUD, 4, 12, buffer, hm.smallerFont);
		}

		{ //health
			const AnimFrame& HeartFrame = level.spriteManager.GetFrame(GetVersionSpecificAnimationID(AnimSets::Pickups), 41, 0);
			for (int i = playerObject.Health - 1; i >= 0; --i)
				HUD.AppendSprite(SpriteMode::Paletted, WINDOW_WIDTH_PIXELS - 12 - i*16, 8, HeartFrame);
		}
		
		{ //lives
			HUD.AppendSprite(SpriteMode::Paletted, 0, WINDOW_HEIGHT_PIXELS, level.spriteManager.GetFrameLimited(GetVersionSpecificAnimationID(AnimSets::Faces), 3 + playerObject.PlayerProperties.CharacterIndex, GameTicks / 6));
			sprintf_s(buffer, "x%u", playerObject.PlayerProperties.Lives);
			WriteText(HUD, 32, WINDOW_HEIGHT_PIXELS - hm.lineHeight - 4, buffer, hm.smallerFont);
		}

		{ //ammo
			const int ammo_xPos = WINDOW_WIDTH_PIXELS - (40 << hm.lineHeightShift);
			const int ammo_yPos = WINDOW_HEIGHT_PIXELS - hm.lineHeight - 4;
			const int ammoID = playerObject.fireType;
			int animID;
			if (ammoID != Weapon::Blaster) {
				animID = AmmoIconAnimIDs[ammoID] - playerObject.PlayerProperties.Powerups[ammoID]; //this is correct for everything other than TNT, which... shouldn't be powered up
			} else switch (playerObject.PlayerProperties.CharacterIndex) {
				case char2SPAZ:
					animID = 19; //wrong, todo
					break;
				//case char2LORI:
//					break;
				default: //jazz etc.
					animID = 18; //wrong, todo
					break;
			}
			HUD.AppendSprite(SpriteMode::Paletted, ammo_xPos - 8, ammo_yPos, level.spriteManager.GetFrameLimited(AnimSets::Ammo, animID, GameTicks >> 2));
		
			if (ammoID == 0) {
				sprintf_s(buffer, "x^");
			} else {
				int ammo = playerObject.PlayerProperties.Ammo[ammoID];
				if (
					ammoID == Weapon::Toaster
#ifdef BETAPEPPER
					|| AmmoID == Weapon::Gun8
#endif
				) ammo >>= 5;
					sprintf_s(buffer, "x%u", ammo);
			}
			WriteText(HUD, ammo_xPos, ammo_yPos, buffer, hm.smallerFont);
		}

#ifdef SHOW_FPS
		{ //fps
			const int info_xPos = WINDOW_WIDTH_PIXELS / 2;
			int info_yPos = WINDOW_HEIGHT_PIXELS - 30;
			sprintf_s(buffer, "fps %3u", std::min(FPS_MAX, FPS));
			WriteText(Sprites, info_xPos, info_yPos, buffer, smallFont);
		}
#endif
	//}

	Layer4Offset = level.Layers[SPRITELAYER].getTransform();
}

void GeneratePointLightingSprite(sf::Uint32* buffer, LightParam radius, LightParam brightness) { //the Generate*LightingSprites were originally written by SE for his HD Lighting project
	const int radiusSquare = radius * radius;
	const int length = radius * 2 + 1;
	for (int i = 0; i < length; i++) {
		const int yDistance = radius - i;
		for (int j = 0; j < length; j++) {
			const int xDistance = radius - j;
			const int distance = int(sqrt(xDistance * xDistance + yDistance * yDistance));
			*buffer++ =
				(distance < radius) ?
				(std::min(((brightness * (radius - distance)) / radius), 255) | 0xFF000000) :
				0;
		}
	}
}
void GenerateNormalLightingSprite(sf::Uint32* buffer, LightParam radius, LightParam brightness) {
}
void GenerateFlickerLightingSprite(sf::Uint32* buffer, LightParam radius, LightParam brightness) {
}
void GenerateRingLightingSprite(sf::Uint32* buffer, LightParam radius, LightParam brightness) {
}

GenerateLightingSprite* GenerateLightingSpriteFunctions[LightType::LAST] = {
	nullptr, GeneratePointLightingSprite, GenerateNormalLightingSprite, GenerateFlickerLightingSprite, GenerateRingLightingSprite
};

//#include "Windows.h"
//#include "Misc.h"
bool DrawObjectToLightBuffer(const BunnyObject& obj) {
	LightParam brightness = (obj.LightType != LightType::Flicker) ? obj.LightIntensity : LightParam(RandFac(255)); //generate at most 255 different possible HD flicker lights and display them basically at random
	const LightHash hash = (obj.LightRadius << 0) | (brightness << 8) | (obj.LightType << 16);
	if (!LightingSpriteProperties.count(hash)) { //this particular lighting image hasn't been predrawn yet, so draw it before rendering it
		AnimFrame& frame = LightingSpriteProperties[hash];
		frame.HotspotX = frame.HotspotY = -sf::Int16(obj.LightRadius);
		const unsigned int spriteDimension = obj.LightRadius * 2 + 1;
		sf::Uint32* buffer = frame.CreateImage(spriteDimension, spriteDimension);
		GenerateLightingSpriteFunctions[obj.LightType](buffer, obj.LightRadius, brightness);
		//OutputDebugStringF(L"%u, %u, %u", obj.LightType, obj.LightRadius, brightness);
		EffectSprites.CreateAndAssignTextureForSingleFrame(frame);
	}

	LightingSprites.AppendSprite(*LightModeAdd, int(obj.PositionX), int(obj.PositionY), LightingSpriteProperties[hash]); //todo laser lights that draw more than a single sprite
	
	return true;
}

void Hook_DrawToWindow(sf::RenderTexture& videoBuffer, sf::RenderWindow& window) {
	LightingBuffer.clear(sf::Color(sf::Uint8(AmbientLightingLevel * 255), 0, 0, 255));
	//LightingBuffer.draw(FullScreenQuad.vertices, 4, sf::Quads, clearAmbientLightingBufferRenderStates);
	LightingBuffer.draw(LightingSprites, Layer4Offset);
	//LasersToDrawOnLightBuffer32.draw(*lightBuffer32); //always draw lasers, and specifically do so after all other lights

	videoBuffer.display();
	LightingStates.texture = &videoBuffer.getTexture();
	window.draw(sf::Sprite(videoBuffer.getTexture()), LightingStates);
	window.draw(HUD);
}
