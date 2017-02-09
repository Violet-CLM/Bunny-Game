#pragma once
#include "SFML/Config.hpp"

typedef sf::Uint8 LightParam;
const LightParam NormalIntensity = 64;
enum LightType {
	None, Point, Normal, Flicker, Ring,
	LAST
};

class BunnyObject;
bool DrawObjectToLightBuffer(const BunnyObject&);