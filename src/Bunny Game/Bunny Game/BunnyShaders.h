#pragma once
#include "Shaders.h"
#include "Drawing.h"

void AppendBunnyShaders(std::vector<std::string>&);

class SpriteModeGem : public SpriteMode {
public:
	SpriteModeGem(sf::Uint8 p) : SpriteMode(Shaders[DefaultShaders::Gem], p) {
		ParamAsFloat = (DefaultPaletteLineNames::Gem + Param) / 8.f; //todo
	}
};