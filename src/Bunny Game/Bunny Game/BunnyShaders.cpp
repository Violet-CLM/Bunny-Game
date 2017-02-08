#include "BunnyShaders.h"
#include "Lattice.h"
#include "Layer.h"
#include "Misc.h"

sf::RenderStates WarpHorizonRenderStates;

std::array<std::string, BunnyShaders::LAST - 1 - BunnyShaders::FIRST> BunnyShaderSources;
void WriteBunnyShaders() {
	BunnyShaderSources = {
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
		}",
	//BunnyShaders::WarpHorizon
		sprintf_z(
			"uniform sampler2D texture256;\
			uniform sampler2D tables;\
			uniform vec4 fadeColor;\
			uniform vec2 offset;\
			\
			void main(void)\
			{\
				float distanceFromVerticalCenter = gl_FragCoord.y - %f;\
				float v109 = 60.0 / (abs(distanceFromVerticalCenter) + 8.0);\
				gl_FragColor = mix(\
					texture2D(tables, vec2(\
						texture2D(texture256, vec2(\
							offset.x + (v109 * (gl_FragCoord.x - %f)) / 256.0,\
							offset.y + (v109 * distanceFromVerticalCenter / -8.0)\
						)).r,\
						0\
					)),\
					fadeColor,\
					texture2D(tables, vec2(\
						abs(distanceFromVerticalCenter) / %f, %f\
					)).r\
				);\
			}", float(WINDOW_HEIGHT_PIXELS/2), float(WINDOW_WIDTH_PIXELS/2), float(WINDOW_HEIGHT_PIXELS/2), TOPALLINE(BunnyPaletteLineNames::TBGFadeIntensity)), //non-LUT version of fade intensity, for speed comparison at some point: "max(1.0 - (distanceProportionalToScreenSize * distanceProportionalToScreenSize * 9.625), 0.0)"
	};


	const sf::BlendMode colorFromSourceAlphaFromDestination(sf::BlendMode::One, sf::BlendMode::Zero, sf::BlendMode::Add, sf::BlendMode::Zero, sf::BlendMode::One, sf::BlendMode::Add);
	WarpHorizonRenderStates.blendMode = colorFromSourceAlphaFromDestination;
}

void Hook_SetupPaletteTables(sf::Texture& tex, const sf::Color* const paletteColors, std::array<sf::Color, COLORSPERPALETTE>& buffer) {
	//Based on the code for generating 16-bit gem color LUTs, which reads indices 0,4,8,12,16 (and ONLY those indices) from the 8-bit LUTs and creates 128-color-long LUTs consisting of four smooth gradients among those five endpoints.
	const static int gemPaletteStopColors[5 * GEM_COLOR_COUNT] = {
		55, 52, 48, 15, 15,
		87, 84, 80, 15, 15,
		39, 36, 32, 15, 15,
		95, 92, 88, 15, 15
	};
	const int* gemPaletteIndexPointer = gemPaletteStopColors;
	sf::Color* DestLUTEntry;
	for (unsigned int gemColorIndex = 0; gemColorIndex < 4; ++gemColorIndex, ++gemPaletteIndexPointer) {
		DestLUTEntry = buffer.data();
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

	DestLUTEntry = buffer.data();
	for (int i = 255; i >= 0; --i, ++DestLUTEntry) { //recreating sub_45E0E0
		const unsigned __int64 v5 = abs(i - 256) << 7;
		DestLUTEntry->r = ~std::min((unsigned int)((4928LL * v5 * v5) >> 33), 255u);
	}
	tex.update((sf::Uint8*)buffer.data(), 256, 1, 0, BunnyPaletteLineNames::TBGFadeIntensity);
}

bool Hook_ShouldTexturedLayerBeUpdated(unsigned int) {
	return false;
}
bool Hook_ShouldTexturedLayerBeRendered(const Layer& layer, sf::RenderTarget& target, sf::RenderStates, unsigned int) {
	const auto renderFrame = Lattice::GetFramesElapsed();
	const auto layerPosition = (&layer)[-3].getPosition(); //layer 5
	const auto layerSpeeds = layer.GetAutoSpeed();
	Shaders[BunnyShaders::WarpHorizon]->setUniform("offset", sf::Vector2f(
		((layerSpeeds.x * renderFrame) - layerPosition.x) / (8*TILEWIDTH),
		((layerSpeeds.y * renderFrame) - layerPosition.y) / (8*TILEWIDTH)
	));
	target.draw(FullScreenQuad.vertices, 4, sf::Quads, WarpHorizonRenderStates);
	return false;
}
