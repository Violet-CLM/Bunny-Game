#pragma once
#include "SFML/Config.hpp"
#include "SFML/Graphics.hpp"

typedef sf::Uint8 LightParam;
const LightParam NormalIntensity = 64;
const float NormalIntensityF = 0.25f;
enum LightType {
	None, Point, Normal, Flicker, Ring,
	LAST
};

class BunnyObject;
bool DrawObjectToLightBuffer(const BunnyObject&);

void InitLighting();
