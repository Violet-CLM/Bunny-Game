#include "PostProcessing.h"
#include "BunnyShaders.h"
#include "Lattice.h"
#include "Layer.h"
#include "Misc.h"

sf::RenderStates WarpHorizonRenderStates, TunnelRenderStates, MenuBGRenderStates;

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
	//BunnyShaders::Palshift
		"uniform sampler2D texture;\
		uniform sampler2D tables;\
		uniform float param;\
		\
		void main(void)\
		{\
			vec4 index = texture2D(texture, gl_TexCoord[0].xy);\
			vec4 pixel = texture2D(tables, vec2(mod(index.r + param, 1.0), 0)); \
			gl_FragColor = vec4(pixel.r, pixel.g, pixel.b, index.a);\
		}",
	//BunnyShaders::Shadow
		"uniform sampler2D texture;\
		\
		void main(void)\
		{\
			gl_FragColor = vec4(0,0,0, texture2D(texture, gl_TexCoord[0].xy).a / 2.0);\
		}",
	//BunnyShaders::Brightness
		"uniform sampler2D texture;\
		uniform sampler2D tables;\
		uniform float param;\
		uniform vec4 brightness;\
		uniform vec4 doublebrightness;\
		\
		void main(void)\
		{\
			vec4 dark = vec4(0, 0, 0, 1.0);\
			vec4 index = texture2D(texture, gl_TexCoord[0].xy);\
			vec4 pixel;\
			if (param <= 1.0) {\
				pixel = mix(texture2D(tables, vec2(index.r, 0)), dark, param); \
			} else {\
				pixel = (texture2D(tables, vec2(index.r, 0)) * param) + (brightness * brightness); \
			}\
			gl_FragColor = vec4(pixel.r, pixel.g, pixel.b, index.a);\
		}",
	//BunnyShaders::SimpleFrozen
		sprintf_z("uniform sampler2D texture;\
		uniform sampler2D tables;\
		\
		void main(void)\
		{\
			vec4 index = texture2D(texture, gl_TexCoord[0].xy);\
			vec4 pixel = texture2D(tables, vec2(texture2D(tables, vec2(index.r, %f)).r, %f)); \
			gl_FragColor = vec4(pixel.r, pixel.g, pixel.b, index.a * 0.5);\
		}", TOPALLINE(BunnyPaletteLineNames::Brightness), TOPALLINE(BunnyPaletteLineNames::BrightnessToFreezeColors)),
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
							offset.y + (v109 * distanceFromVerticalCenter / 8.0)\
						)).r,\
						0\
					)),\
					fadeColor,\
					texture2D(tables, vec2(\
						abs(distanceFromVerticalCenter) / %f, %f\
					)).r\
				);\
			}", float(WINDOW_HEIGHT_PIXELS/2), float(WINDOW_WIDTH_PIXELS/2), float(WINDOW_HEIGHT_PIXELS/2), TOPALLINE(BunnyPaletteLineNames::TBGFadeIntensity)), //non-LUT version of fade intensity, for speed comparison at some point: "max(1.0 - (distanceProportionalToScreenSize * distanceProportionalToScreenSize * 9.625), 0.0)"
	//BunnyShaders::Tunnel
		sprintf_z("uniform sampler2D texture256;\
		uniform sampler2D tables;\
		uniform vec4 fadeColor;\
		uniform vec2 offset;\
		uniform float spiral;\
		\
		void main(void)\
		{\
			float relative_x = (%f - gl_FragCoord.x);\
			float relative_y = (gl_FragCoord.y - %f);\
			float depth = 64.0 / (relative_x * relative_x + relative_y * relative_y) * (%f - gl_FragCoord.y);\
			float spiralAngleDepth = spiral * depth;\
			gl_FragColor = mix(\
				texture2D(tables, vec2(\
					texture2D(texture256, vec2(\
						atan(relative_x, relative_y) / 6.28318531 + 0.5 + offset.x + (spiralAngleDepth * spiralAngleDepth),\
						depth + offset.y\
					)).r,\
					0\
				)),\
				fadeColor,\
				clamp(depth / 4.0, 0.0, 1.0)\
			);\
		}", float(WINDOW_WIDTH_PIXELS/2), float(WINDOW_HEIGHT_PIXELS/2), float(WINDOW_HEIGHT_PIXELS + WINDOW_HEIGHT_PIXELS/2)),
	//BunnyShaders::MenuBG
		sprintf_z("uniform sampler2D texture128;\
		uniform sampler2D texture32;\
		uniform sampler2D texture16;\
		uniform sampler2D tables;\
		uniform mat4 transform128;\
		uniform vec4 transform32;\
		uniform mat4 transform16;\
		const vec2 resize128 = vec2(128.0, 128.0);\
		const vec2 resize32 = vec2(32.0, 32.0);\
		const vec2 resize16 = vec2(16.0, 16.0);\
		const vec4 translate16 = vec4(8.0, 8.0, 0, 0);\
		\
		void main(void)\
		{\
			vec4 position = gl_FragCoord - vec4(%f, %f, 0, 0);\
			vec4 index = texture2D(texture128, (position *= transform128).xy / resize128);\
			index = mix(texture2D(texture32, (position += transform32).xy / resize32), index, index.g);\
			index = mix(texture2D(texture16, ((mod(position, 32.0) - translate16) * transform16).xy / resize16), index, index.g);\
			gl_FragColor = texture2D(tables, vec2(index.r, 0));\
		}", float(WINDOW_WIDTH_PIXELS/2), float(WINDOW_HEIGHT_PIXELS/2)),
	//BunnyShaders::ClearAmbientLightingBuffer
		"uniform sampler2D remapping;\
		uniform sampler2D texture;\
		uniform float newIntensity;\
		\
		void main(void)\
		{\
			gl_FragColor = texture2D(remapping, vec2(texture2D(texture, gl_TexCoord[0].xy).r, newIntensity)); \
		}",
	//BunnyShaders::ClearAmbientLightingBufferMenu
		sprintf_z("void main(void)\
		{\
			gl_FragColor = vec4(%f, 0, 0, 0.5); \
		}", NormalIntensityF),
	//BunnyShaders::BlurAmbientLightingBuffer
		sprintf_z("uniform sampler2D texture;\
			\
			void main(void)\
			{\
				gl_FragColor = vec4((\
					texture2D(texture, gl_TexCoord[0].xy).r +\
					texture2D(texture, gl_TexCoord[0].xy + vec2(%f,0)).r +\
					texture2D(texture, gl_TexCoord[0].xy - vec2(%f,0)).r +\
					texture2D(texture, gl_TexCoord[0].xy + vec2(0,%f)).r +\
					texture2D(texture, gl_TexCoord[0].xy - vec2(0,%f)).r\
				) / 5.0, 0,0, 1.0);\
			}", float(8) / WINDOW_WIDTH_PIXELS, float(8) / WINDOW_WIDTH_PIXELS, float(8) / WINDOW_HEIGHT_PIXELS, float(8) / WINDOW_HEIGHT_PIXELS),
	//BunnyShaders::ApplyAmbientLightingToVideo
		"uniform sampler2D texture;\
		uniform sampler2D lightBuffer;\
		uniform vec4 dark;\
		\
		void main(void)\
		{\
			vec4 color = texture2D(texture, gl_TexCoord[0].xy);\
			vec4 light = texture2D(lightBuffer, gl_TexCoord[0].xy);\
			float lightIntensity = light.r * 4.0;\
			if (lightIntensity <= 1.0) {\
				gl_FragColor = mix(dark, color, lightIntensity); \
			} else {\
				float smallerIntensity = lightIntensity - 1.0;\
				gl_FragColor = (color * lightIntensity) + (smallerIntensity * smallerIntensity); \
			}\
		}",
	};


	const sf::BlendMode colorFromSourceAlphaFromDestination(sf::BlendMode::One, sf::BlendMode::Zero, sf::BlendMode::Add, sf::BlendMode::Zero, sf::BlendMode::One, sf::BlendMode::Add);
	WarpHorizonRenderStates.blendMode = colorFromSourceAlphaFromDestination;
	TunnelRenderStates.blendMode = colorFromSourceAlphaFromDestination;
	MenuBGRenderStates.blendMode = colorFromSourceAlphaFromDestination;
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
	for (int i = COLORSPERPALETTE-1; i >= 0; --i, ++DestLUTEntry) { //recreating sub_45E0E0
		const unsigned __int64 v5 = abs(i - COLORSPERPALETTE) << 7;
		DestLUTEntry->r = ~std::min((unsigned int)((4928LL * v5 * v5) >> 33), 255u);
	}
	tex.update((sf::Uint8*)buffer.data(), COLORSPERPALETTE, 1, 0, BunnyPaletteLineNames::TBGFadeIntensity);

	DestLUTEntry = buffer.data();
	for (int i = 0; i < COLORSPERPALETTE; ++i, ++DestLUTEntry) {
		const auto color = paletteColors[i];
		DestLUTEntry->r = (7499 * color.b + 38446 * color.g + 19591 * color.r) >> 16;
		DestLUTEntry->a = UINT8_MAX;
	}
	tex.update((sf::Uint8*)buffer.data(), COLORSPERPALETTE, 1, 0, BunnyPaletteLineNames::Brightness);

	DestLUTEntry = buffer.data();
	for (int i = 0; i < COLORSPERPALETTE; ++i, ++DestLUTEntry) {
		*DestLUTEntry = sf::Color(i >> 1, std::min(32 + (i << 1), 255), std::min(i * i + 32, 255), 128);
	}
	tex.update((sf::Uint8*)buffer.data(), COLORSPERPALETTE, 1, 0, BunnyPaletteLineNames::BrightnessToFreezeColors);

	SpriteModeFrozen = SpriteMode(Shaders[BunnyShaders::SimpleFrozen], 0);
	SpriteModeJustHit = SpriteMode(Shaders[DefaultShaders::SingleColorPaletted], 15);
}

bool Hook_ShouldTexturedLayerBeUpdated(unsigned int) {
	return false;
}
bool Hook_ShouldTexturedLayerBeRendered(const Layer& layer, sf::RenderTarget& target, sf::RenderStates, unsigned int textureMode) {
	const auto renderFrame = Lattice::GetFramesElapsed();
	const auto layerPosition = (&layer)[-3].getPosition(); //layer 5
	const auto layerSpeeds = layer.GetAutoSpeed();
	const bool isTunnel = textureMode & 1;
	const float horizontalDiver = float((8 * TILEWIDTH) << (isTunnel << 1));
	Shaders[!isTunnel ? BunnyShaders::WarpHorizon : BunnyShaders::Tunnel]->setUniform("offset", sf::Vector2f(
		((layerSpeeds.x * renderFrame) - layerPosition.x) / horizontalDiver,
		((layerSpeeds.y * renderFrame) - layerPosition.y) / (8*TILEWIDTH)
	));
	target.draw(FullScreenShape.vertices, PrimitiveCount, OpenGLPrimitive, !isTunnel ? WarpHorizonRenderStates : TunnelRenderStates);
	return false;
}

SpriteMode SpriteModeFrozen, SpriteModeJustHit;
