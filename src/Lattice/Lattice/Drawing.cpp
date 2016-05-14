#include "Drawing.h"
#include "Misc.h"
#include "Windows.h"

quad PossibleQuadOrientations[2 * 2 * 2];
quad FullScreenQuad;

void GeneratePaletteTexture(sf::Texture& tex, const sf::Uint8* palette)
{
	tex.create(COLORSPERPALETTE, pallineNUMBEROFPALLINES);
	tex.update(palette, COLORSPERPALETTE, 1, 0, pallineNORMALPALETTE);

	for (int i = 0; i < shader_LAST; ++i) {
		shaders[i]->setParameter("tables", tex); //cannot be used more than once...
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

void InitCreateShaders()
{
	for (int i = 0; i < shader_LAST; ++i) {
		const std::string& shaderSource = shaderSources[i];
		if (shaderSource.empty()) //empty (unimplemented) shader string
			continue;
		sf::Shader* shader = shaders[i] = new sf::Shader();
		if (shader->loadFromMemory(shaderSource, sf::Shader::Fragment))
			shader->setParameter("texture", sf::Shader::CurrentTexture);
	}
}

void VertexCollection::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
	states.texture = Texture;
	target.draw(Vertices.data(), Vertices.size(), sf::Quads, states);
}

void VertexCollection::AppendQuad(quad& q)
{
	q.appendTo(Vertices);
}

bool VertexCollection::Matches(sf::Texture* otherTexture) const
{
	return Texture == otherTexture;
}
