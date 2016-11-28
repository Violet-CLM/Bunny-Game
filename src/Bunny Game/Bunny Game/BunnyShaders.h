#pragma once
#include <array>
#include "Shaders.h"
#include "Drawing.h"
#include "BunnyMisc.h"

namespace BunnyPaletteLineNames {
	enum {
		FIRST = DefaultPaletteLineNames::LAST - 1,
		Gem, __lastgem = Gem + GEM_COLOR_COUNT-1,
								  //pallineBRIGHTNESS, pallineBRIGHTNESSTOFREEZECOLORS, pallineMENUPLAYERSPRITEMAPPING, pallineTBGFADEINTENSITY, pallineHEATEFFECTOFFSETS, pallinePLAYERS, pallineGEMS = pallinePLAYERS + 32,
		LAST
	};
}
namespace BunnyShaders {
	enum {
		FIRST = DefaultShaders::LAST - 1,
		Gem,
		LAST
	};
}

extern std::vector<std::string> BunnyShaderSources;
PaletteTableSetupFunction GeneratePaletteTextureBunnyEdition;

class SpriteModeGem : public SpriteMode {
public:
	SpriteModeGem(sf::Uint8 p) : SpriteMode(Shaders[BunnyShaders::Gem], p) {
		ParamAsFloat = (BunnyPaletteLineNames::Gem + Param) / 8.f;
	}
};