#pragma once
#include "SFML/Config.hpp"
#include "SFML/Graphics.hpp"
#include "Tile.h"
#include "Shaders.h"
#include "Constants.h"

struct quad {
	sf::Vertex vertices[4];
	int TextureID;
	quad() {
		vertices[0].position.x = 0;			vertices[0].position.y = 0;
		vertices[1].position.x = TILEWIDTH;	vertices[1].position.y = 0;
		vertices[2].position.x = TILEWIDTH;	vertices[2].position.y = TILEHEIGHT;
		vertices[3].position.x = 0;			vertices[3].position.y = TILEHEIGHT;
		for (int i = 0; i < 4; ++i)
			vertices[i].texCoords = vertices[i].position;
	}
	quad(float w, float h) {
		setDimensions(w, h);
	}
	/*void useTexCoordsOfSprite(int frameID) {
		sf::Vector2<float>* const frameTexCoords = SpritePropertyList[frameID].texCoords;
		for (int i = 0; i < 4; ++i)
			vertices[i].texCoords = frameTexCoords[i];
	}*/
	/*quad(int frameID, short angle, int scaleX, int scaleY) {
		useTexCoordsOfSprite(frameID);
		if (scaleX < 0)
			flipHorizontally();
		if (scaleY < 0)
			flipVertically();

		const Tframe* const frame = &frames[frameID];
		const int left = (scaleX < 0) ? -(frame->width + frame->hotSpotX) : frame->hotSpotX;
		const int top = (scaleY < 0) ? -(frame->height + frame->hotSpotY) : frame->hotSpotY;
		const int right = frame->width + left;
		const int bottom = frame->height + top;

		scaleX = abs(scaleX);
		scaleY = abs(scaleY);
		const int sine = sinTable(angle) << 6;
		const int cosine = cosTable(angle) << 6;
		const int sineTimesInverseScaleX = sine / scaleX;
		const int sineTimesInverseScaleY = sine / scaleY;
		const int cosineTimesInverseScaleX = cosine / scaleX;
		const int cosineTimesInverseScaleY = cosine / scaleY;

		const int leftSine = left * sineTimesInverseScaleX;
		const int leftCosine = left * cosineTimesInverseScaleX;
		const int rightSine = right * sineTimesInverseScaleX;
		const int rightCosine = right * cosineTimesInverseScaleX;
		const int topSine = top * sineTimesInverseScaleY;
		const int topCosine = top * cosineTimesInverseScaleY;
		const int bottomSine = bottom * sineTimesInverseScaleY;
		const int bottomCosine = bottom * cosineTimesInverseScaleY;

		vertices[0].position.x = float((topSine + leftCosine) >> FIXFAC);
		vertices[0].position.y = float((topCosine - leftSine) >> FIXFAC);
		vertices[1].position.x = float((topSine + rightCosine) >> FIXFAC);
		vertices[1].position.y = float((topCosine - rightSine) >> FIXFAC);
		vertices[2].position.x = float((bottomSine + rightCosine) >> FIXFAC);
		vertices[2].position.y = float((bottomCosine - rightSine) >> FIXFAC);
		vertices[3].position.x = float((bottomSine + leftCosine) >> FIXFAC);
		vertices[3].position.y = float((bottomCosine - leftSine) >> FIXFAC);
	}*/

	/*quad(int frameID, int width, int height, bool flippedX, bool flippedY) {
		setDimensions((float)width, (float)height);
		useTexCoordsOfSprite(frameID);

		if (flippedX)
			flipHorizontally();
		if (flippedY)
			flipVertically();
	}*/
	quad(const quad& tileQuad, int tileOrientation, unsigned int quadrant = 4) {
		extern quad PossibleQuadOrientations[];

		this->operator=(tileQuad);
		if (tileOrientation) { //no reason to vacuously alter the vertices for non-flipped, non-rotated tiles
			quad* const positions = &PossibleQuadOrientations[tileOrientation];
			for (int i = 0; i < 4; ++i)
				vertices[i].position = positions->vertices[i].texCoords;
		}
		if (quadrant < 4) { //only a quarter tile
			for (int i = 0; i < 4; ++i) {
				sf::Vector2f* const position = &vertices[i].position;
				position->x /= 2;
				position->y /= 2;
			}
			if (quadrant < 2) { //top
				vertices[2].texCoords.y -= TILEHEIGHT / 2;
				vertices[3].texCoords.y -= TILEHEIGHT / 2;
			}
			else {
				vertices[0].texCoords.y += TILEHEIGHT / 2;
				vertices[1].texCoords.y += TILEHEIGHT / 2;
			}
			if ((quadrant & 1) == 0) { //left
				vertices[1].texCoords.x -= TILEWIDTH / 2;
				vertices[2].texCoords.x -= TILEWIDTH / 2;
			}
			else {
				vertices[0].texCoords.x += TILEWIDTH / 2;
				vertices[3].texCoords.x += TILEWIDTH / 2;
			}
		}
	}
	quad(const quad* const quadsPerTile, Tile tile, unsigned int quadrant = 4) : quad(quadsPerTile[tile.ID], tile.raw >> 12, quadrant) {}
	/*quad(const SpriteProperties* const properties) {
		TextureID = properties->TextureID;
		for (int i = 0; i < 4; ++i)
			vertices[i].texCoords = properties->texCoords[i];
		setDimensions(vertices[1].texCoords.x - vertices[0].texCoords.x, vertices[2].texCoords.y - vertices[1].texCoords.y);
	}*/
	quad& operator=(const quad& other) {
		memcpy(this, &other, sizeof(quad));
		return *this;
	}
	quad(const quad& other) {
		this->operator=(other);
	}

	void setDimensions(float w, float h) {
		vertices[0].position.x = 0;	vertices[0].position.y = 0;
		vertices[1].position.x = w;	vertices[1].position.y = 0;
		vertices[2].position.x = w;	vertices[2].position.y = h;
		vertices[3].position.x = 0;	vertices[3].position.y = h;
	}
	void flipHorizontally() {
		float swap = vertices[0].texCoords.x;
		vertices[0].texCoords.x = vertices[1].texCoords.x;
		vertices[1].texCoords.x = swap;
		swap = vertices[2].texCoords.x;
		vertices[2].texCoords.x = vertices[3].texCoords.x;
		vertices[3].texCoords.x = swap;
	}
	void flipVertically() {
		float swap = vertices[0].texCoords.y;
		vertices[0].texCoords.y = vertices[3].texCoords.y;
		vertices[3].texCoords.y = swap;
		swap = vertices[2].texCoords.y;
		vertices[2].texCoords.y = vertices[1].texCoords.y;
		vertices[1].texCoords.y = swap;
	}
	void rotateClockwise() {
		const sf::Vector2f swap = vertices[0].texCoords;
		vertices[0].texCoords = vertices[3].texCoords;
		vertices[3].texCoords = vertices[2].texCoords;
		vertices[2].texCoords = vertices[1].texCoords;
		vertices[1].texCoords = swap;
	}

	void positionTexCoordsAt(int x, int y) {
		for (int i = 0; i < 4; ++i) {
			vertices[i].texCoords.x += x * TILEWIDTH;
			vertices[i].texCoords.y += y * TILEHEIGHT;
		}
	}
	void positionPositionAt(int x, int y) {
		for (int i = 0; i < 4; ++i) {
			vertices[i].position.x += x;
			vertices[i].position.y += y;
		}
	}
	void useTexCoordsOfTile(int tileID) {
		positionTexCoordsAt(tileID % TILESETDIMENSIONINTILES, (tileID % TILESPERTEXTURE) / TILESETDIMENSIONINTILES);
	}
	void appendTo(VertexVector& vertexVector) {
		//OuputDebugStringF("%f, %f, %f, %f, %f, %f, %f, %f", vertices[0].texCoords.x, vertices[0].texCoords.y, vertices[2].texCoords.x, vertices[2].texCoords.y, vertices[0].position.x, vertices[0].position.y, vertices[2].position.x, vertices[2].position.y);
		for (int i = 0; i < 4; ++i) {
			vertexVector.push_back(vertices[i]);
		}
	}
};

struct SpriteMode {
protected:
	sf::Shader* Shader;
	float ParamAsFloat;
	sf::Uint8 Param;
public:
	SpriteMode() : Shader(nullptr), ParamAsFloat(0), Param(0) {}
	SpriteMode(sf::Shader* s, sf::Uint8 p) : Shader(s), Param(p) {
		ParamAsFloat = p / 256.0f;
	}
	bool operator==(const SpriteMode& other) const {
		return other.Shader == Shader && other.Param == Param;
	}
	virtual sf::Shader* GetShader() const;

	static SpriteMode Normal, Paletted;
};
class AnimFrame;
class VertexCollection : public sf::Drawable, public sf::Transformable {
private:
	sf::Texture* Texture;
	SpriteMode Mode;
	VertexVector Vertices;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
	VertexCollection() {}
	VertexCollection(sf::Texture* t, const SpriteMode& m) : Texture(t), Mode(m) {}
	void AppendQuad(quad&);
	bool Matches(const sf::Texture* const, const SpriteMode&) const; //todo more options
};
class VertexCollectionQueue : public sf::Drawable {
protected:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
	std::vector<VertexCollection> Collections;

	void DrawQuad(quad&, sf::Texture*, const SpriteMode&);
	void DrawSprite(const SpriteMode&, int, int, const AnimFrame&, bool = false, bool = false);
	void DrawRectangle(const SpriteMode&, int, int, int, int, sf::Uint8);
	void DrawPixel(const SpriteMode&, int, int, sf::Uint8);
};

void GeneratePaletteTexture(sf::Texture&, const sf::Uint8*, PaletteTableSetupFunction, unsigned int);
void GenerateTilesetTextures(sf::Texture**, const char*, const sf::Uint32*, const sf::Uint8*, unsigned int);

void InitPopulateTextureArrays();
void InitCreateShaders(std::vector<sf::Shader*>&, const std::vector<std::string>&);

extern sf::Texture* PaletteTexture;