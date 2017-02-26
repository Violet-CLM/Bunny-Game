#pragma once
#include "SFML/Config.hpp"
#include "SFML/Graphics.hpp"
#include "Drawing.h"

typedef sf::Uint8 LightParam;
const LightParam NormalIntensity = 64;
const float NormalIntensityF = 0.25f;
extern unsigned int AmbientLightingLevel;
extern void ClearLightingBuffer(float = NormalIntensityF);

namespace LightType { enum LightType {
	None, Point, Normal, Flicker, Ring, Player,
	LAST
}; }

extern VertexCollectionQueue LightingSprites;

class BunnyObject;
void DrawLightToLightBuffer(LightType::LightType, LightParam, LightParam, sf::Vector2f);
void DrawObjectToLightBuffer(const BunnyObject&);

void InitLighting();
