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
typedef void GenerateLightingSprite(sf::Color*, unsigned int, unsigned int);

float AmbientLightingLevel = 0.125f;

sf::Transform Layer4Offset;
SpriteManager EffectSprites;
sf::Texture ClearAmbientLightingBufferColorLUT;
sf::RenderTexture LightingBuffer[2];
sf::RenderStates LightingStates, ClearAmbientLightingBufferRenderStates, BlurAmbientLightingBufferRenderStates;
VertexCollectionQueue LightingSprites, HUD;
SpriteMode *LightModeAdd, *LightModeAlpha, *LightModeMax;
std::unordered_map<LightHash, AnimFrame> LightingSpriteProperties;

void InitLighting() {
	for (int i = 0; i < 2; ++i)
		LightingBuffer[i].create(WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS);
	LightingStates = sf::RenderStates(sf::BlendNone, sf::Transform::Identity, nullptr, Shaders[BunnyShaders::ApplyAmbientLightingToVideo]);
	ClearAmbientLightingBufferRenderStates = sf::RenderStates(sf::BlendNone, sf::Transform::Identity, nullptr, Shaders[BunnyShaders::ClearAmbientLightingBuffer]);
	BlurAmbientLightingBufferRenderStates = sf::RenderStates(sf::BlendNone, sf::Transform::Identity, nullptr, Shaders[BunnyShaders::BlurAmbientLightingBuffer]);

	LightModeAdd = new SpriteMode(Shaders[DefaultShaders::Normal], 0, sf::BlendAdd);
	LightModeAlpha = new SpriteMode(Shaders[DefaultShaders::Normal], 0, sf::BlendAlpha);
	LightModeMax = new SpriteMode(Shaders[DefaultShaders::Normal], 0, sf::BlendMode(sf::BlendMode::One, sf::BlendMode::One, sf::BlendMode::Equation::Max));
	{
	std::array<sf::Color, COLORSPERPALETTE * COLORSPERPALETTE> ClearLUTImage;
	for (int previousIntensity = 0; previousIntensity < COLORSPERPALETTE; ++previousIntensity) //recreate GeneralGlobals->lightShadeTransitionMappingLightValue
		for (int newBaseIntensity = 0; newBaseIntensity < COLORSPERPALETTE; ++newBaseIntensity) {
			auto& newIntensity = ClearLUTImage[previousIntensity + newBaseIntensity * COLORSPERPALETTE].r;
			if (previousIntensity + 4 < newBaseIntensity) {
				newIntensity = std::min(127, previousIntensity + 4);
			} else {
				if (previousIntensity - 6 > newBaseIntensity)
					newIntensity = std::max(0, previousIntensity - 6);
				else
					newIntensity = newBaseIntensity;
			}
		}
	ClearAmbientLightingBufferColorLUT.create(COLORSPERPALETTE,COLORSPERPALETTE);
	ClearAmbientLightingBufferColorLUT.update((const sf::Uint8*)ClearLUTImage.data());
	}
	Shaders[BunnyShaders::ApplyAmbientLightingToVideo]->setUniform("dark", sf::Glsl::Vec4());
	Shaders[BunnyShaders::ApplyAmbientLightingToVideo]->setUniform("lightBuffer", LightingBuffer[1].getTexture());
	Shaders[BunnyShaders::ClearAmbientLightingBuffer]->setUniform("remapping", ClearAmbientLightingBufferColorLUT);
	BlurAmbientLightingBufferRenderStates.texture = &LightingBuffer[0].getTexture();
	ClearAmbientLightingBufferRenderStates.texture = &LightingBuffer[1].getTexture();
}

void Hook_ClearSpriteQueues() {
	HUD.Clear();
	LightingSprites.Clear();
	for (const auto& it : Players)
		DrawObjectToLightBuffer(*it.Object); //draw players FIRST
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
	WriteCharacter writeCharToHUD = GetWriteCharacterFunction(HUD);

	const auto& play = Players[0]; //no splitscreen support at present
	//for (const auto& play : Players) if (play.Object != nullptr) {
		const auto& playerObject = *play.Object;

		if (playerObject.HelpStringCounter < 0) {
			const auto result = playerObject.HelpStringCounter + 6*AISPEED;
			int x = TtextAppearance::DefaultCenterAlign;
			static TtextAppearance helpStringAppearance(TtextAppearance::defaultSpin);
			if (result >= AISPEED) {
				if (result >= 5*AISPEED) { //fade out
					x -= (playerObject.HelpStringCounter + AISPEED) * (playerObject.HelpStringCounter + AISPEED) / 20;
					helpStringAppearance.inverseAmplitude = 0x8000 - 220 * (playerObject.HelpStringCounter + AISPEED);
				} else
					helpStringAppearance.inverseAmplitude = 0x8000;
			} else { //fade in
				x += (AISPEED - result) * (AISPEED - result) / 20;
				helpStringAppearance.inverseAmplitude = 0x8000 - 220 * (AISPEED - result);
			}
			WriteText(writeCharToHUD, x,10, playerObject.HelpString, smallFont, helpStringAppearance, GameTicks);
		}

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
			WriteText(writeCharToHUD, 4, 12, buffer, hm.smallerFont);
		}

		{ //health
			const AnimFrame& HeartFrame = level.spriteManager.GetFrame(GetVersionSpecificAnimationID(AnimSets::Pickups), 41, 0);
			for (int i = playerObject.Health - 1; i >= 0; --i)
				HUD.AppendSprite(SpriteMode::Paletted, WINDOW_WIDTH_PIXELS - 12 - i*16, 8, HeartFrame);
		}
		
		{ //lives
			HUD.AppendSprite(SpriteMode::Paletted, 0, WINDOW_HEIGHT_PIXELS, level.spriteManager.GetFrameLimited(GetVersionSpecificAnimationID(AnimSets::Faces), 3 + playerObject.PlayerProperties.CharacterIndex, GameTicks / 6));
			sprintf_s(buffer, "x%u", playerObject.PlayerProperties.Lives);
			WriteText(writeCharToHUD, 32, WINDOW_HEIGHT_PIXELS - hm.lineHeight - 4, buffer, hm.smallerFont);
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
			WriteText(writeCharToHUD, ammo_xPos, ammo_yPos, buffer, hm.smallerFont);
		}

#ifdef SHOW_FPS
		{ //fps
			const int info_xPos = WINDOW_WIDTH_PIXELS / 2;
			int info_yPos = WINDOW_HEIGHT_PIXELS - 30;
			sprintf_s(buffer, "fps %3u", std::min(FPS_MAX, Lattice::FPS));
			WriteText(HUD, info_xPos, info_yPos, buffer, smallFont);
		}
#endif
	//}

	Layer4Offset = level.Layers[SPRITELAYER].getTransform();
}

void GeneratePointLightingSprite(sf::Color* buffer, unsigned int radius, unsigned int brightness) { //the Generate*LightingSprites were originally written by SE for his HD Lighting project
	const int radiusSquare = radius * radius;
	const int length = radius * 2 + 1;
	for (int i = 0; i < length; i++) {
		const int yDistance = radius - i;
		for (int j = 0; j < length; j++) {
			const int xDistance = radius - j;
			const unsigned int distance = unsigned int(sqrt(xDistance * xDistance + yDistance * yDistance));
			*buffer++ =
				(distance < radius) ?
				sf::Color(std::min(((brightness * (radius - distance)) / radius), 255u), 0,0, 0xFFu) :
				sf::Color::Transparent;
		}
	}
}
#define LIGHT32CIRCLEFADESIZE 28
void GenerateAreaLightingSprite(sf::Color* buffer, unsigned int radius, unsigned int brightness, unsigned int opacity) {
	const int radiusSquare = radius * radius;
	const int length = radius * 2 + 1;
	const int whiteRange = (radius - LIGHT32CIRCLEFADESIZE) * (radius - LIGHT32CIRCLEFADESIZE);
	const int grayRange = radiusSquare - whiteRange;
	const sf::Color innerBrightness(brightness, 0, 0, opacity);
	for (int i = 0; i < length; ++i) {
		const int yDistance = radius - i;
		for (int j = 0; j < length; ++j, ++buffer) {
			const int xDistance = radius - j;
			const int distanceSquare = xDistance * xDistance + yDistance * yDistance;
			if (distanceSquare < radiusSquare) {
				if (distanceSquare < whiteRange)
					*buffer = innerBrightness;
				else
					*buffer = sf::Color(brightness, 0,0, std::min((opacity * (radiusSquare - distanceSquare) / grayRange), 255u));
			} //else
				//*buffer = sf::Color::Transparent;
		}
	}
}
void GenerateNormalLightingSprite(sf::Color* buffer, unsigned int radius, unsigned int brightness) {
	GenerateAreaLightingSprite(buffer, radius, brightness, 255);
}
void GenerateBrightLightingSprite(sf::Color* buffer, unsigned int radius, unsigned int brightness) {
	GenerateAreaLightingSprite(buffer, radius, brightness, 127);
}

void GenerateFlickerLightingSprite(sf::Color* buffer, unsigned int radius, unsigned int brightness) {
	const int radiusSquare = radius * radius;
	const int length = radius * 2 + 1;
	const int whiteRange = (radius - LIGHT32CIRCLEFADESIZE) * (radius - LIGHT32CIRCLEFADESIZE);
	const int grayRange = radiusSquare - whiteRange;
	sf::Color* bufferStart = buffer;
	//noise
	const int noiseLength = ((length + 7) / 8);
	std::vector<sf::Uint8> bufferNoise;
	for (int i = 0; i < noiseLength * noiseLength; ++i)
		bufferNoise.push_back((RandFac(7) ? (RandFac(3) << 5) : 0));
	//despeckle
	for (int y = 1; y < noiseLength-1; ++y)
		for (int x = 1; x < noiseLength-1; ++x) {
				bufferNoise[x + y*noiseLength] = ((
					bufferNoise[x + y*noiseLength] +
					bufferNoise[x-1 + y*noiseLength] +
					bufferNoise[x+1 + y*noiseLength] +
					bufferNoise[x + (y-1)*noiseLength] +
					bufferNoise[x + (y+1)*noiseLength]
				) / 5 + 15) & ~31;
		}
	//resize pixels to 8x8
	for (int i = 0; i < length; ++i) {
		const int yDistance = radius - i;
		const int yDistanceDivided = yDistance * yDistance / 200;
		for (int j = 0; j < length; ++j, ++buffer) {
			const int xDistance = radius - j;
			const int distanceSquare = xDistance * xDistance + yDistance * yDistance;
			if (distanceSquare < radiusSquare) {
				*buffer = sf::Color(
					bufferNoise[i / 8 * noiseLength + j / 8] + yDistanceDivided, 0, 0,
					(distanceSquare < whiteRange) ?
						63 :
						63 * (radiusSquare - distanceSquare) / grayRange
				);
			} //else
				//*buffer = sf::Color::Transparent;
		}
	}
	//blur
	for (int i = 0; i < 2; ++i) { //number of times to blur
		buffer = bufferStart;
		for (int y = 0; y < length; ++y) {
			for (int x = 0; x < length; ++x, ++buffer) {
				if (y > 1 && x > 1 && y < length-2 && x < length-2 && buffer->a != 0) {
					const sf::Uint32 oldR = buffer->r;
					buffer->r = (
						oldR + oldR + oldR +
						buffer[-1].r +
						buffer[1].r +
						buffer[-length*2-2].r + //sample one pixel away orthogonally and two diagonally, giving a rounded effect
						buffer[-length].r +
						buffer[-length*2+2].r +
						buffer[length*2-2].r +
						buffer[length].r +
						buffer[length*2+2].r
					) / 11;
				}
			}
		}
	}
}
void GenerateRingLightingSprite(sf::Color* buffer, unsigned int radius, unsigned int brightness) {
	const int length = radius * 2 + 1;
	const int whiteRange = (radius - 8) * (radius - 8);
	const int grayRange = (radius * radius) - whiteRange;
	for (int i = 0; i < length; ++i) {
		const int yDistance = radius - i;
		for (int j = 0; j < length; ++j, ++buffer) {
			const int xDistance = radius - j;
			const int absoluteDistance = grayRange - abs(whiteRange - (xDistance * xDistance + yDistance * yDistance));
			if (absoluteDistance > 0) {
				*buffer = sf::Color(brightness, 0,0, std::min((127 * absoluteDistance / grayRange), 127)); //alpha WAS 255, but that looks too bright
			} //else
				//*buffer = sf::Color::Transparent;
		}
	}
}
void GeneratePlayerLightingSprite(sf::Color* buffer, unsigned int radius, unsigned int brightness) {
	const int radiusSquare = radius * radius;
	const int length = radius * 2 + 1;
	const int whiteRange = (radius - LIGHT32CIRCLEFADESIZE) * (radius - LIGHT32CIRCLEFADESIZE);
	const int grayRange = radiusSquare - whiteRange;
	const sf::Color innerBrightness(brightness, 0,0, 255);
	for (int i = 0; i < length; ++i) {
		const int yDistance = radius - i;
		for (int j = 0; j < length; ++j, ++buffer) {
			const int xDistance = radius - j;
			const int distanceSquare = xDistance * xDistance + yDistance * yDistance;
			if (distanceSquare < radiusSquare) {
				if (distanceSquare < whiteRange)
					*buffer = innerBrightness;
				else
					*buffer = sf::Color(std::min((brightness * (radiusSquare - distanceSquare) / grayRange), 255u), 0,0, 255);
			} //else
				//*buffer = sf::Color::Transparent;
		}
	}
}

GenerateLightingSprite* GenerateLightingSpriteFunctions[LightType::LAST] = {
	nullptr, GeneratePointLightingSprite, GenerateNormalLightingSprite, GenerateFlickerLightingSprite, GenerateRingLightingSprite, GeneratePlayerLightingSprite
};

//#include "Windows.h"
//#include "Misc.h"
void DrawLightToLightBuffer(LightType::LightType type, LightParam radius, LightParam brightness, sf::Vector2f position) {
	if (type == LightType::Flicker) brightness = LightParam(RandFac(31)); //generate at most 32 different possible HD flicker lights and display them basically at random
	const LightHash hash = (radius << 0) | (brightness << 8) | (type << 16);
	if (!LightingSpriteProperties.count(hash)) { //this particular lighting image hasn't been predrawn yet, so draw it before rendering it
		AnimFrame& frame = LightingSpriteProperties[hash];
		frame.HotspotX = frame.HotspotY = -sf::Int16(radius);
		const unsigned int spriteDimension = radius * 2 + 1;
		sf::Uint32* buffer = frame.CreateImage(spriteDimension, spriteDimension);
		GenerateLightingSpriteFunctions[type]((sf::Color*)buffer, radius, brightness);
		//OutputDebugStringF(L"%u, %u, %u", spriteDimension, obj.LightRadius, brightness);
		EffectSprites.CreateAndAssignTextureForSingleFrame(frame);
	}

	LightingSprites.AppendSprite(
		*(
			(type == LightType::Point || type == LightType::Ring) ?
				LightModeAdd :
				(type != LightType::Player) ?
					LightModeAlpha :
					LightModeMax
		),
		int(position.x), int(position.y),
		LightingSpriteProperties[hash]
	); //todo laser lights that draw more than a single sprite

}
void DrawObjectToLightBuffer(const BunnyObject& obj) {
	DrawLightToLightBuffer(obj.LightType, obj.LightRadius, obj.LightIntensity, sf::Vector2f(obj.PositionX, obj.PositionY));
}

void Hook_DrawToWindow(sf::RenderTexture& videoBuffer, sf::RenderWindow& window) {
	/*static unsigned int lastFrames = 0; //change light at random every second; good for testing light fading
	const auto currentRenderFrame = Lattice::GetFramesElapsed();
	if (currentRenderFrame > lastFrames + AISPEED) {
		lastFrames = currentRenderFrame;
		AmbientLightingLevel = RandFac(127) / 255.f;
	}*/

	Shaders[BunnyShaders::ClearAmbientLightingBuffer]->setUniform("newIntensity", AmbientLightingLevel);
	LightingBuffer[0].draw(FullScreenQuad.vertices, 4, sf::Quads, ClearAmbientLightingBufferRenderStates); //fade the previous version of the buffer onto the new base intensity
	LightingBuffer[0].draw(LightingSprites, Layer4Offset); //draw all the new light sources
	//laser beam goes here
	LightingBuffer[1].draw(FullScreenQuad.vertices, 4, sf::Quads, BlurAmbientLightingBufferRenderStates); //blur everything
	//laser shield goes here

	videoBuffer.display();
	LightingStates.texture = &videoBuffer.getTexture();
	window.draw(FullScreenQuadNonFlipped.vertices, 4, sf::Quads, LightingStates);
	window.draw(HUD);
}
