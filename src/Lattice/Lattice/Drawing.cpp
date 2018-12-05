#include <array>
#include "Lattice.h"
#include "Drawing.h"
#include "Misc.h"
#include "Windows.h"
#include "Resources.h"
#include "LatticeHooks.h"

sf::Texture* PaletteTexture;

quad PossibleQuadOrientations[2 * 2 * 2];
quad FullScreenQuad, FullScreenQuadNonFlipped;
#ifndef SUPPORT_QUADS
triangleQuad FullScreenTriangleQuad, FullScreenTriangleQuadNonFlipped;
#endif

void GeneratePaletteTexture(sf::Texture& tex, const sf::Uint8* palette)
{
	const sf::Color* const paletteColors = (sf::Color*)palette;
	
	PaletteTexture = &tex;

	unsigned int paletteHeight = 1;
	while (paletteHeight < PaletteLineCount)	//smallest power of 2 containing the number of palette lines
		paletteHeight <<= 1;					//there are cleverer, faster, harder-to-read ways to do this, but it's a onetime startup operation so I don't really care
	std::array<sf::Color, COLORSPERPALETTE> buffer;
	tex.create(COLORSPERPALETTE, paletteHeight);
	tex.update(palette, COLORSPERPALETTE, 1, 0, DefaultPaletteLineNames::Palette);
	for (int i = 0; i < COLORSPERPALETTE; ++i)
		buffer[i].r = i;
	tex.update((sf::Uint8*)buffer.data(), COLORSPERPALETTE, 1, 0, DefaultPaletteLineNames::XPosToIndex);

	Hook_SetupPaletteTables(tex, paletteColors, buffer);

	for (auto& shader : Shaders)
		shader->setUniform("tables", tex); //cannot be used more than once...
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
			if (shader->loadFromMemory(shaderSource, sf::Shader::Fragment)) {
				if (shaderSource.find("uniform sampler2D texture") != std::string::npos)
					shader->setUniform("texture", sf::Shader::CurrentTexture);
			} else
				ShowErrorMessage((L"Error compiling shader: " + std::wstring(shaderSource.begin(), shaderSource.end())).c_str());
			shaders.push_back(shader);
		}
	}
}

void VertexCollection::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
	states.texture = Texture;
	Mode->Apply(states);
	target.draw(Vertices.data(), Vertices.size(), OpenGLPrimitive, states);
}
void VertexCollectionQueue::draw(sf::RenderTarget & target, sf::RenderStates states) const
{
	for (const auto& it : Collections)
		target.draw(it, states);
}


void VertexCollection::AppendQuad(quad& q)
{
	q.appendTo(Vertices);
}

bool VertexCollection::Matches(const sf::Texture* const otherTexture, const SpriteMode& otherMode) const
{
	return Texture == otherTexture && *Mode == otherMode;
}

void SpriteMode::Apply(sf::RenderStates& states) const
{
	Shader->setUniform("param", ParamAsFloat);
	states.shader = Shader;
	states.blendMode = BlendMode;
}

void VertexCollectionQueue::AppendQuad(quad& q, sf::Texture* texture, const SpriteMode& spriteMode)
{
	if (Collections.empty() || !Collections.back().Matches(texture, spriteMode))
		Collections.emplace_back(texture, spriteMode);
	Collections.back().AppendQuad(q);
}
void VertexCollectionQueue::AppendSprite(const SpriteMode& mode, int x, int y, const AnimFrame& sprite, bool flipX, bool flipY)
{
	quad repositionedQuad(sprite.Quad);
	if (flipX)
		repositionedQuad.flipHorizontally();
	if (flipY)
		repositionedQuad.flipVertically();
	repositionedQuad.positionPositionAt(x + (!flipX ? sprite.HotspotX : -(sprite.Width + sprite.HotspotX)), y + (!flipY ? sprite.HotspotY : -(sprite.Height + sprite.HotspotY)));
	AppendQuad(repositionedQuad, sprite.Texture, mode);
}
void VertexCollectionQueue::AppendRectangle(const SpriteMode& mode, int x, int y, int width, int height, sf::Uint8 color)
{
	quad rectangleQuad((float)width, (float)height);
	const sf::Vector2f texCoords(static_cast<float>(color), static_cast<float>(DefaultPaletteLineNames::XPosToIndex));
	rectangleQuad.positionPositionAt(x, y);
	for (int i = 0; i < 4; ++i)
		rectangleQuad.vertices[i].texCoords = texCoords;
	AppendQuad(rectangleQuad, PaletteTexture, mode);
}
void VertexCollectionQueue::AppendPixel(const SpriteMode& mode, int x, int y, sf::Uint8 color)
{
	return AppendRectangle(mode, x, y, 1, 1, color);
}
void VertexCollectionQueue::AppendResizedSprite(const SpriteMode& mode, int x, int y, const AnimFrame& sprite, float scaleX, float scaleY)
{
	if (scaleX == 0.f || scaleY == 0.f)
		return;

	quad repositionedQuad(sprite.Quad);
	const bool flippedX = scaleX < 0;
	const bool flippedY = scaleY < 0;
	scaleX = abs(scaleX);
	scaleY = abs(scaleY);
	if (flippedX)
		repositionedQuad.flipHorizontally();
	if (flippedY)
		repositionedQuad.flipVertically();
	repositionedQuad.setDimensions(scaleX * sprite.Width, scaleY * sprite.Height);
	repositionedQuad.positionPositionAt(
		x + int(scaleX * (!flippedX ? sprite.HotspotX : (1 - sprite.HotspotX - sprite.Width))),
		y + int(scaleY * (!flippedY ? sprite.HotspotY : (1 - sprite.HotspotY - sprite.Height)))
	);
	AppendQuad(repositionedQuad, sprite.Texture, mode);
}
void VertexCollectionQueue::AppendRotatedSprite(const SpriteMode& mode, int x, int y, const AnimFrame& sprite, float angle, float scaleX, float scaleY)
{
	if (scaleX == 0.f || scaleY == 0.f)
		return;

	quad repositionedQuad(sprite.Quad);
	if (scaleX < 0)
		repositionedQuad.flipHorizontally();
	if (scaleY < 0)
		repositionedQuad.flipVertically();

	const int left = (scaleX < 0) ? -(sprite.Width + sprite.HotspotX) : sprite.HotspotX;
	const int top = (scaleY < 0) ? -(sprite.Height + sprite.HotspotY) : sprite.HotspotY;
	const int right = sprite.Width + left;
	const int bottom = sprite.Height + top;

	scaleX = abs(scaleX);
	scaleY = abs(scaleY);
	const float sine = sin(angle);
	const float cosine = cos(angle);
	const float sineTimesInverseScaleX = sine / scaleX;
	const float sineTimesInverseScaleY = sine / scaleY;
	const float cosineTimesInverseScaleX = cosine / scaleX;
	const float cosineTimesInverseScaleY = cosine / scaleY;

	const float leftSine = left * sineTimesInverseScaleX;
	const float leftCosine = left * cosineTimesInverseScaleX;
	const float rightSine = right * sineTimesInverseScaleX;
	const float rightCosine = right * cosineTimesInverseScaleX;
	const float topSine = top * sineTimesInverseScaleY;
	const float topCosine = top * cosineTimesInverseScaleY;
	const float bottomSine = bottom * sineTimesInverseScaleY;
	const float bottomCosine = bottom * cosineTimesInverseScaleY;

	repositionedQuad.vertices[0].position = sf::Vector2f(topSine + leftCosine, topCosine - leftSine);
	repositionedQuad.vertices[1].position = sf::Vector2f(topSine + rightCosine, topCosine - rightSine);
	repositionedQuad.vertices[2].position = sf::Vector2f(bottomSine + rightCosine, bottomCosine - rightSine);
	repositionedQuad.vertices[3].position = sf::Vector2f(bottomSine + leftCosine, bottomCosine - leftSine);

	repositionedQuad.positionPositionAt(x, y);
	AppendQuad(repositionedQuad, sprite.Texture, mode);
}

void VertexCollectionQueue::Clear() {
	Collections.resize(0);
}
