#pragma once
#include <memory>
#include "SFML/Config.hpp"
#include "SFML/Graphics.hpp"
#include "Tile.h"
#include "Shaders.h"
#include "Constants.h"

struct quad;
extern quad PossibleQuadOrientations[];
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
	quad(float w, float h, int x, int y) {
		setDimensions(w, h);
		positionPositionAt(x, y);
	}

	quad(const quad& tileQuad, int tileOrientation, unsigned int quadrant = 4) {
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
extern quad FullScreenQuad, FullScreenQuadNonFlipped;

struct SpriteMode {
protected:
	sf::Shader* Shader;
	float ParamAsFloat;
	sf::Uint8 Param;
	sf::BlendMode BlendMode;
public:
	SpriteMode() : Shader(nullptr), ParamAsFloat(0), Param(0), BlendMode(sf::BlendAlpha) {}
	SpriteMode(sf::Shader* s, sf::Uint8 p, sf::BlendMode b = sf::BlendAlpha) : Shader(s), Param(p), BlendMode(b) {
		ParamAsFloat = p / 256.0f;
	}
	bool operator==(const SpriteMode& other) const {
		return other.Shader == Shader && other.Param == Param && other.BlendMode == BlendMode;
	}
	virtual void Apply(sf::RenderStates&) const;
	virtual SpriteMode* clone() const { return new SpriteMode(*this); }

	static SpriteMode Normal, Paletted;
};
template <class Derived>
class SpriteModeDerivationHelper: public SpriteMode { //http://stackoverflow.com/questions/5731217/how-to-copy-create-derived-class-instance-from-a-pointer-to-a-polymorphic-base-c
public:
	using SpriteMode::SpriteMode;
	virtual SpriteMode* clone() const
	{
		return new Derived(static_cast<const Derived&>(*this)); // call the copy ctor.
	}
};
#define DerivedSpriteMode(name) class name : public SpriteModeDerivationHelper <name>

class AnimFrame;
class VertexCollection : public sf::Drawable, public sf::Transformable {
private:
	sf::Texture* Texture;
	std::unique_ptr<SpriteMode> Mode;
	VertexVector Vertices;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
	VertexCollection() {}
	VertexCollection(sf::Texture* t, const SpriteMode& m) : Texture(t), Mode(m.clone()) {}
	void AppendQuad(quad&);
	bool Matches(const sf::Texture* const, const SpriteMode&) const; //todo more options
};
class VertexCollectionQueue : public sf::Drawable {
private:
	std::vector<VertexCollection> Collections;
protected:
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
	void AppendQuad(quad&, sf::Texture*, const SpriteMode&);
	void AppendSprite(const SpriteMode&, int, int, const AnimFrame&, bool = false, bool = false);
	void AppendRectangle(const SpriteMode&, int, int, int, int, sf::Uint8);
	void AppendPixel(const SpriteMode&, int, int, sf::Uint8);
	void AppendResizedSprite(const SpriteMode&, int, int, const AnimFrame&, float,float);
	void AppendRotatedSprite(const SpriteMode&, int, int, const AnimFrame&, float, float=1.f,float=1.f);

	void Clear();
};

void GeneratePaletteTexture(sf::Texture&, const sf::Uint8*);
void GenerateTilesetTextures(sf::Texture**, const char*, const sf::Uint32*, const sf::Uint8*, unsigned int);

void InitPopulateTextureArrays();
void InitCreateShaders(std::vector<sf::Shader*>&, const std::vector<std::string>&);

extern sf::Texture* PaletteTexture;