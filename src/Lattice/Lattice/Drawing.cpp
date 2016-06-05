#include "Drawing.h"
#include "Misc.h"
#include "Windows.h"

sf::Texture* PaletteTexture;

quad PossibleQuadOrientations[2 * 2 * 2];
quad FullScreenQuad;

//#define PALLINEHEIGHT (1.f/float(pallineNUMBEROFPALLINES)) //texture positions in GLSL are not 0-255 (or 0-63 or whatever) but 0.0 to 1.0. This is therefore the height of a single line in the "tables" texture, and e.g. (float(pallineBRIGHTNESS) * PALLINEHEIGHT) points to line pallineBRIGHTNESS in GLSL coordinates
//#define TOPALLINE(A) (float(A) * PALLINEHEIGHT) //a macro for the above location purpose, to be used while writing shaders
void GeneratePaletteTexture(sf::Texture& tex, const sf::Uint8* palette)
{
	const sf::Color* const paletteColors = (sf::Color*)palette;
	
	PaletteTexture = &tex;

	unsigned int paletteHeight = 8;// 1; //todo
	//while (paletteHeight < Shaders.size())
		//paletteHeight <<= 1;
	sf::Color buffer[COLORSPERPALETTE];
	tex.create(COLORSPERPALETTE, paletteHeight);
	tex.update(palette, COLORSPERPALETTE, 1, 0, DefaultPaletteLineNames::Palette);
	for (int i = 0; i < COLORSPERPALETTE; ++i)
		buffer[i].r = i;
	tex.update((sf::Uint8*)buffer, COLORSPERPALETTE, 1, 0, DefaultPaletteLineNames::XPosToIndex);

	//Based on the code for generating 16-bit gem color LUTs, which reads indices 0,4,8,12,16 (and ONLY those indices) from the 8-bit LUTs and creates 128-color-long LUTs consisting of four smooth gradients among those five endpoints.
	const static int gemPaletteStopColors[5*4] = {
		55, 52, 48, 15, 15,
		87, 84, 80, 15, 15,
		39, 36, 32, 15, 15,
		95, 92, 88, 15, 15
	};
	const int* gemPaletteIndexPointer = gemPaletteStopColors;
	for (unsigned int gemColorIndex = 0; gemColorIndex < 4; ++gemColorIndex, ++gemPaletteIndexPointer) {
		sf::Color* DestLUTEntry = buffer;
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
		tex.update((sf::Uint8*)buffer, COLORSPERPALETTE, 1, 0, DefaultPaletteLineNames::Gem + gemColorIndex);
	}

	for (int i = 0; i < DefaultShaders::LAST; ++i) {
		Shaders[i]->setParameter("tables", tex); //cannot be used more than once...
	}
}

void GenerateTilesetTextures(sf::Texture** TileImages, const char* TileTypes, const sf::Uint32* ImageAddresses, const sf::Uint8* Images, unsigned int TileCount)
{
	sf::Uint8* textureInput = (sf::Uint8*)malloc(TILESETDIMENSION * TILESETDIMENSION * BYTESPER32BITPIXEL);
	for (unsigned int textureID = 0, firstTileIDOfTexture = 0; firstTileIDOfTexture < TileCount; ++textureID, firstTileIDOfTexture += TILESPERTEXTURE) {
		sf::Texture& texture = *(TileImages[textureID] = new sf::Texture());
		texture.create(TILESETDIMENSION, TILESETDIMENSION);

		for (unsigned int tileID = 0; tileID < TILESPERTEXTURE; ++tileID) {
			const unsigned int combinedTileID = tileID + firstTileIDOfTexture;
			if (combinedTileID >= TileCount)
				break;
			const sf::Uint8* tileInput = Images + *ImageAddresses++;
			sf::Uint8* tileOutput = textureInput + ((tileID % TILESETDIMENSIONINTILES) * TILEWIDTH + (tileID / TILESETDIMENSIONINTILES) * TILEHEIGHT * TILESETDIMENSION) * BYTESPER32BITPIXEL;
			for (int y = 0; y < TILEHEIGHT; ++y, tileOutput += TILESETDIMENSION * BYTESPER32BITPIXEL)
				for (int x = 0; x < TILEWIDTH; ++x) {
					const sf::Uint8 tilePixel = *tileInput++;
					((sf::Uint32*)tileOutput)[x] = tilePixel ? ((TileTypes[combinedTileID] != 1 ? 0xFF000000u : 0xBF000000u) | tilePixel) : 0; //0x00000000 for pixel color 0 (transparent); 0xBF000000u for translucent tiles (or would I rather 7F? hmmmm)
				}
		}

		texture.update(textureInput);
		//texture.copyToImage().saveToFile("C:\\Games\\Jazz2\\diam1texture.png");
	}
	free(textureInput);
}

void InitPopulateTextureArrays() {
	for (int i = 0; i < 8; ++i) {
		if (i & (TILE_HFLIPPED >> 12))
			PossibleQuadOrientations[i].flipHorizontally();
		if (i & (TILE_VFLIPPED >> 12))
			PossibleQuadOrientations[i].flipVertically();
		if (i & (TILE_ROTATED >> 12))
			PossibleQuadOrientations[i].rotateClockwise();
	}
	/*
	for (int layerID = 0; layerID < 8; ++layerID)
		for (int tileTypeID = 0; tileTypeID < DISTINCTLYDRAWNTILETYPECOUNT; ++tileTypeID)
			for (int textureID = 0; textureID < NUMBEROFTILESETTEXTURES; ++textureID)
				new (&layerVertexArrays[layerID * NUMBEROFVERTEXARRAYSPERLAYER + tileTypeID * NUMBEROFTILESETTEXTURES + textureID]) vertexArrayAndItsProperties(tilesetTextures32[textureID], tileTypesToSpriteTypes[tileTypeID], 0);
	*/
}

SpriteMode SpriteMode::Normal, SpriteMode::Paletted;
void InitCreateShaders(std::vector<sf::Shader*>& shaders, const std::vector<std::string>& shaderSources)
{
	for (auto& shaderSource : shaderSources) {
		if (shaderSource.empty()) //empty (unimplemented) shader string
			shaders.push_back(nullptr);
		else {
			sf::Shader* shader = new sf::Shader();
			if (shader->loadFromMemory(shaderSource, sf::Shader::Fragment))
				shader->setParameter("texture", sf::Shader::CurrentTexture);
			else
				ShowErrorMessage((L"Error compiling shader:" + std::wstring(shaderSource.begin(), shaderSource.end())).c_str());
			shaders.push_back(shader);
		}
	}
}

void VertexCollection::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
	states.texture = Texture;
	states.shader = Mode.GetShader();
	target.draw(Vertices.data(), Vertices.size(), sf::Quads, states);
}

void VertexCollection::AppendQuad(quad& q)
{
	q.appendTo(Vertices);
}

bool VertexCollection::Matches(const sf::Texture* const otherTexture, const SpriteMode& otherMode) const
{
	return Texture == otherTexture && Mode == otherMode;
}

sf::Shader* SpriteMode::GetShader() const
{
	Shader->setParameter("param", ParamAsFloat);
	return Shader;
}
