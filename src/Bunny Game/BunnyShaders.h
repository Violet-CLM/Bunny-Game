#pragma once
#include <array>
#include "Shaders.h"
#include "Drawing.h"
#include "BunnyMisc.h"

#define NUMBEROFPALLINES 8 //smallest power of 2 greater or equal to BunnyPaletteLineNames::LAST-1; has to be manually calculated in order to be built into the shaders
#define PALLINEHEIGHT (1.f/float(NUMBEROFPALLINES)) //texture positions in GLSL are not 0-255 (or 0-63 or whatever) but 0.0 to 1.0. This is therefore the height of a single line in the "tables" texture, and e.g. (float(pallineBRIGHTNESS) * PALLINEHEIGHT) points to line pallineBRIGHTNESS in GLSL coordinates
#define TOPALLINE(A) (float(A) * PALLINEHEIGHT) //a macro for the above location purpose, to be used while writing shaders

namespace BunnyPaletteLineNames {
	enum {
		FIRST = DefaultPaletteLineNames::LAST - 1,
		Gem, __lastgem = Gem + GEM_COLOR_COUNT-1,
		TBGFadeIntensity,
								  //pallineBRIGHTNESS, pallineBRIGHTNESSTOFREEZECOLORS, pallineMENUPLAYERSPRITEMAPPING, pallineHEATEFFECTOFFSETS, pallinePLAYERS, pallineGEMS = pallinePLAYERS + 32,
		LAST
	};
}
namespace BunnyShaders {
	enum {
		FIRST = DefaultShaders::LAST - 1,
		Gem, Palshift, Shadow, Brightness,
		WarpHorizon, Tunnel, MenuBG,
		ClearAmbientLightingBuffer, ClearAmbientLightingBufferMenu, BlurAmbientLightingBuffer, ApplyAmbientLightingToVideo, 
		LAST
	};
}

extern std::array<std::string, BunnyShaders::LAST - 1 - BunnyShaders::FIRST> BunnyShaderSources;
void WriteBunnyShaders();

extern sf::RenderStates WarpHorizonRenderStates, TunnelRenderStates, MenuBGRenderStates;

class SpriteModeGem : public SpriteMode {
public:
	SpriteModeGem(sf::Uint8 p) : SpriteMode(Shaders[BunnyShaders::Gem], p) {
		ParamAsFloat = (BunnyPaletteLineNames::Gem + Param) / 8.f;
	}
};