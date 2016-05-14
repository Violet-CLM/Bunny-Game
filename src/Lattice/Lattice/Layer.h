#pragma once
#include "SFML/Config.hpp"
#include "Tile.h"
#include "Drawing.h"
#include "Tileset.h"
#include "Constants.h"

class Level;
struct Layer : public sf::Drawable, public sf::Transformable { //order of properties is arbitrary, not connected to any native JJ2 code
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
	//TbgFadeColor FadeColor;
	bool IsTextured;
	sf::Uint8 TextureMode;
	bool ParallaxStars;
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
	std::vector<VertexCollection> SpriteQueue;

	bool ForceUpdate;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

public:
	Layer();
	Layer(Level*, const char*, Word*, const WordID*&, int);

	sf::Int32 Width; sf::Int32 RealWidth; sf::Int32 RoundedWidth; sf::Int32 WidthPixels;
	sf::Int32 Height; sf::Int32 HeightPixels;

	Tile SetTile(int x, int y, Tile nu);
	Tile GetTile(int x, int y) const;

	bool MaskedPixel(int, int) const;
	unsigned int MaskedHLine(int, int, int) const;
	unsigned int MaskedVLine(int, int, int) const;

	void Update(unsigned int, unsigned int, sf::Vector2f);
	void ClearSpriteQueue();
	void DrawQuad(quad&, sf::Texture*);
};
