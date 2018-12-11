#pragma once
#include <vector>
#include <string>
#include "SFML/Graphics.hpp"
#include "Constants.h"

//LUTs stored in the GPU in the texture "tables", corresponding to paletteTexture32 in the CPU. Each row is 256 colors wide and is a single LUT (or a space where one could be added later). They may be rearranged freely--the shaders in drawing32shaders.cpp are generated using these enum values to determine which row to use for any given lookup.
namespace DefaultPaletteLineNames {
	enum {
		Palette, XPosToIndex,
		LAST
	};
}
namespace DefaultShaders {
	enum {
		Normal, Paletted, SingleColorPaletted,
		LAST
	};
}

extern std::vector<std::string> ShaderSources;
extern std::vector<sf::Shader*> Shaders;
extern unsigned int PaletteLineCount;