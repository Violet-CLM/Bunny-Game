#pragma once
#include <array>
#include "SFML/Config.hpp"
#include "Tile.h"
#include "Drawing.h"
#include "Tileset.h"
#include "Constants.h"

class Level;
#ifdef PARTICLECOUNT
#include "Particles.h"
#endif
struct Layer : public VertexCollectionQueue, public sf::Transformable { //order of properties is arbitrary, not connected to any native JJ2 code
private:
	//sf::Uint32 LayerID;
	float SpeedX; float SpeedY;
	float PositionX; float PositionY;
	float LastPositionX; float LastPositionY;
	float AutoSpeedX; float AutoSpeedY;
	//sf::Uint32 XOffset; sf::Uint32 YOffset;
	//sf::Uint32 RotationAngle; sf::Uint32 RotationRadiusMultiplier;
	//sf::Uint32 PersonalXPosition[MAXLOCALPLAYERS]; sf::Uint32 PersonalYPosition[MAXLOCALPLAYERS];
	//sf::Uint32 PersonalXInternalOffset[MAXLOCALPLAYERS]; sf::Uint32 PersonalYInternalOffset[MAXLOCALPLAYERS];
	//sf::Uint32 DrawVerticalPixelOffset; //Range: 0 to 31. //Jerry
	//sf::Uint32 DrawWordPixelOffset; //Range: -127 to 0. //Jerry
	bool IsTextured;
	sf::Uint8 TextureMode;
	bool IsHeightTiled;
	bool IsWidthTiled;
	bool LimitVisibleRegion;
	bool IsUsed;
	//bool IsUsable;
	bool HasSprites;

	//char ENDOFVALUETYPES; //don't memset or memcpy past here

	//spriteQueue_t SpriteQueue[MAXLOCALPLAYERS];
	Level* LevelPtr;
	Word* Dictionary;
	const WordID* WordMap;
	//sf::Usf::Uint3216 *VisibleWords;//[256];
	//sf::Usf::Uint3232* VisibleTileVisibilityMasks;//[1024];

	VertexVector Vertices[NUMBEROFTILESETTEXTURES];

	bool ForceUpdate;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
	bool DrawTileToVertexArrays(VertexVector* vertices,unsigned int x,unsigned int y) const; //returns true if tile is animated

public:
	Layer();
	Layer(Level*, const char*, Word*, const WordID*&, int);

	sf::Int32 Width; sf::Int32 RealWidth; sf::Int32 RoundedWidth; sf::Int32 WidthPixels;
	sf::Int32 Height; sf::Int32 HeightPixels;
	sf::Vector2f GetSpeed() const;
	sf::Vector2f GetAutoSpeed() const;

	Tile SetTile(int x, int y, Tile nu);
	Tile GetTile(int x, int y) const;

	bool MaskedPixel(int, int) const;
	unsigned int MaskedHLine(int, int, int) const;
	unsigned int MaskedVLine(int, int, int) const;

#ifdef PARTICLECOUNT
	std::array<Particle, PARTICLECOUNT> Particles;
#endif

	std::vector<sf::Texture> AdditionalTextures;
	bool Get_IsTextured() const { return IsTextured; }
	void MakeTexture(sf::RenderTexture&) const;
	sf::Color FadeColor;
	bool ParallaxStars;

	void Update(unsigned int, sf::Vector2f);
};
