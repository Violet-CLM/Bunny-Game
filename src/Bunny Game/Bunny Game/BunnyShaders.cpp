#include "BunnyShaders.h"
#include "Misc.h"

std::vector<std::string> BunnyShaderSources = {
		//BunnyShaders::Gem
	"uniform sampler2D texture;\
	uniform sampler2D tables;\
	uniform float param;\
	\
	void main(void)\
	{\
		vec4 index = texture2D(texture, gl_TexCoord[0].xy);\
		vec4 pixel = texture2D(tables, vec2(mod(index.r, 0.5), param)); \
		gl_FragColor = vec4(pixel.r, pixel.g, pixel.b, index.a * 0.75);\
	}"
};

void GeneratePaletteTextureBunnyEdition(sf::Texture& tex, const sf::Color* const paletteColors, std::array<sf::Color, COLORSPERPALETTE>& buffer) {
	//Based on the code for generating 16-bit gem color LUTs, which reads indices 0,4,8,12,16 (and ONLY those indices) from the 8-bit LUTs and creates 128-color-long LUTs consisting of four smooth gradients among those five endpoints.
	const static int gemPaletteStopColors[5 * GEM_COLOR_COUNT] = {
		55, 52, 48, 15, 15,
		87, 84, 80, 15, 15,
		39, 36, 32, 15, 15,
		95, 92, 88, 15, 15
	};
	const int* gemPaletteIndexPointer = gemPaletteStopColors;
	for (unsigned int gemColorIndex = 0; gemColorIndex < 4; ++gemColorIndex, ++gemPaletteIndexPointer) {
		sf::Color* DestLUTEntry = buffer.data();
		for (unsigned int gemGradientPosition = 0; gemGradientPosition < 4; ++gemGradientPosition) {
			sf::Color firstColor = paletteColors[*gemPaletteIndexPointer];
			sf::Color secondColor = paletteColors[*++gemPaletteIndexPointer];

			unsigned int red = firstColor.r << 3, deltaRed = (secondColor.r << 3) - red; //the JJ2 code multiplies by 6 instead of by 8, but that's because gems are drawn with only 75% opacity. In 32-bit that's done in the shader with a 0.75 alpha, so the values here can be speedily multiplied by the full 8.
			unsigned int green = firstColor.g << 3, deltaGreen = (secondColor.g << 3) - green;
			unsigned int blue = firstColor.b << 3, deltaBlue = (secondColor.b << 3) - blue;
			red <<= 5;
			green <<= 5;
			blue <<= 5;

			for (unsigned int gradientSubPosition = 0; gradientSubPosition < 32; ++gradientSubPosition) {
				DestLUTEntry->r = red >> 8;		red += deltaRed;
				DestLUTEntry->g = green >> 8;	green += deltaGreen;
				DestLUTEntry->b = blue >> 8;	blue += deltaBlue;
				++DestLUTEntry;
			}
		}
		tex.update((sf::Uint8*)buffer.data(), COLORSPERPALETTE, 1, 0, BunnyPaletteLineNames::Gem + gemColorIndex);
	}
}
