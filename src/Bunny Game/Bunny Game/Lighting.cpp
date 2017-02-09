#include "Lighting.h"
#include "BunnyObject.h"
#include "BunnyMisc.h"
#include "Resources.h"

typedef unsigned int LightHash;
typedef void GenerateLightingSprite(sf::Uint32*, LightParam, LightParam);

SpriteManager EffectSprites;
VertexCollectionQueue LightingSprites;
std::unordered_map<LightHash, AnimFrame> LightingSpriteProperties;

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
	LightingSprites.AppendSprite(SpriteMode::Normal, int(obj.PositionX), int(obj.PositionY),  LightingSpriteProperties[hash]); //todo flicker, laser lights that both draw more than a single sprite
	
	return true;
}
