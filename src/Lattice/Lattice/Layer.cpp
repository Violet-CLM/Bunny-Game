#include "Windows.h"
#include "SFML/Graphics.hpp"
#include "Misc.h"
#include "Layer.h"
#include "Level.h"

Layer::Layer()
{
	PositionX = LastPositionX = PositionY = LastPositionY = 0;
	ForceUpdate = true; //always needs to be updated first time it's drawn
}

Layer::Layer(Level* level, const char* data1Ptr, Word* data3Ptr, const WordID*& data4Ptr, int layerID) : Layer()
{
	const sf::Uint32 layerMiscProperties = ((sf::Uint32*)data1Ptr)[layerID];
	IsWidthTiled = !!(layerMiscProperties & 1);
	IsHeightTiled = !!(layerMiscProperties & 2);
	LimitVisibleRegion = !!(layerMiscProperties & 4);
	IsTextured = !!(layerMiscProperties & 8);
	ParallaxStars = !!(layerMiscProperties & 16);
	data1Ptr += sizeof(sf::Uint32) * LEVEL_LAYERCOUNT; //LayerMiscProperties
	data1Ptr += sizeof(sf::Uint8) * LEVEL_LAYERCOUNT; //"Type"
	IsUsed = /*IsUsable = */((bool*)data1Ptr)[layerID];
	data1Ptr += sizeof(bool) * LEVEL_LAYERCOUNT; //DoesLayerHaveAnyTiles
	Width = ((int*)data1Ptr)[layerID];
	WidthPixels = Width * TILEWIDTH;
	data1Ptr += sizeof(sf::Uint32) * LEVEL_LAYERCOUNT; //LayerWidth
	RealWidth = ((int*)data1Ptr)[layerID];
	RoundedWidth = (RealWidth + 3) / TILES_IN_WORD;
	data1Ptr += sizeof(sf::Uint32) * LEVEL_LAYERCOUNT; //LayerRealWidth
	Height = ((int*)data1Ptr)[layerID];
	HeightPixels = Height * TILEHEIGHT;
	data1Ptr += sizeof(sf::Uint32) * LEVEL_LAYERCOUNT; //LayerHeight
	data1Ptr += sizeof(sf::Uint32) * LEVEL_LAYERCOUNT; //"LayerZAxis"
	data1Ptr += sizeof(sf::Uint8) * LEVEL_LAYERCOUNT; //"DetailLevel"
	//XOffset = ((int*)data1Ptr)[layerID];
	data1Ptr += sizeof(sf::Uint32) * LEVEL_LAYERCOUNT; //"WaveX" (now XOffset)
	//YOffset = ((int*)data1Ptr)[layerID];
	data1Ptr += sizeof(sf::Uint32) * LEVEL_LAYERCOUNT; //"WaveY" (now YOffset)
	SpeedX = ((sf::Int32*)data1Ptr)[layerID] / 65536.f;
	data1Ptr += sizeof(sf::Uint32) * LEVEL_LAYERCOUNT; //LayerXSpeed
	SpeedY = ((sf::Int32*)data1Ptr)[layerID] / 65536.f;
	data1Ptr += sizeof(sf::Uint32) * LEVEL_LAYERCOUNT; //LayerYSpeed
	AutoSpeedX = ((sf::Int32*)data1Ptr)[layerID] / 65536.f;
	data1Ptr += sizeof(sf::Uint32) * LEVEL_LAYERCOUNT; //LayerAutoXSpeed
	AutoSpeedY = ((sf::Int32*)data1Ptr)[layerID] / 65536.f;
	data1Ptr += sizeof(sf::Uint32) * LEVEL_LAYERCOUNT; //LayerAutoYSpeed
	TextureMode = data1Ptr[layerID];
	data1Ptr += sizeof(sf::Uint8) * LEVEL_LAYERCOUNT; //LayerTextureMode
	//FadeColor = *(TbgFadeColor*)(data1Ptr + 3 * layerID);

	//OutputDebugStringF(L"%d, %d", Width, Height);

	LevelPtr = level;
	Dictionary = data3Ptr;
	WordMap = data4Ptr;
	if (IsUsed) data4Ptr += RoundedWidth * Height; //passed by reference, so this prepares the pointer for the next layer



	//RotationAngle = layerRotationAngle[layerID];						//I guess?
	//RotationRadiusMultiplier = layerRotationRadiusMultiplier[layerID];	//better than 0, probably

	/*AllocateVisibilityArrays();

	if (!IsUsable) //don't bother allocating a sf::UintMap
		return;
	sf::UintMap = (sf::Uint16*)malloc(layerSize * sizeof(sf::Uint16));
	sf::Uint16* sf::UintMapPtr = sf::UintMap;
	for (unsigned int i = 0; i < layerSize; ++i) { //find sf::Uints in dictionary, and if not present, add them
		sf::Uint16 sf::UintID = *data4Ptr++;
		const TileDictionaryEntry& sf::Uint = data3Ptr[sf::UintID];
		auto it = std::find(TileDictionaryV.begin(), TileDictionaryV.end(), sf::Uint);
		if (it == TileDictionaryV.end()) { //no match, so this is a totally new sf::Uint
			TileDictionaryV.push_back(sf::Uint);
			sf::UintID = (sf::Uint16)TileDictionaryV.size() - 1;
			(*(int*)(versionTSF ? 0x560728 : 0x5509C8))++;
		}
		else
			sf::UintID = std::distance(TileDictionaryV.begin(), it);
		*sf::UintMapPtr++ = sf::UintID;
	}
	*(TileDictionaryEntry**)(&LevelGlobals->tileDictionary) = TileDictionaryV.data(); //in case the capacity changed and the memory therefore moved
	if (TileDictionaryV.size() > 0xFFFFu) {
		extern void AngelEcho(const std::string&, bool timestamp = false);
		AngelEcho("ANGELSCRIPT: Too many sf::Uints in tile cache, errors may occur");
	}
	*/
}

Tile Layer::SetTile(int x, int y, Tile nu)
{
	Dictionary[WordMap[x / TILES_IN_WORD + y * RoundedWidth]][x % TILES_IN_WORD] = nu;
	ForceUpdate = true; //just assume it's on-screen instead of wasting too much time finding out
	return nu;
}
Tile Layer::GetTile(int x, int y) const
{
	return Dictionary[WordMap[x / TILES_IN_WORD + y * RoundedWidth]][x % TILES_IN_WORD];
}

bool Layer::MaskedPixel(int x, int y) const
{
	if (x < 0)
		x = 0;
	else if (x >= WidthPixels)
		x = WidthPixels - 1;
	if (y < 0)
		y = 0;
	else if (y >= HeightPixels)
		y = HeightPixels - 1;
	return LevelPtr->TilesetPtr->MaskedPixel(GetTile(x / TILEWIDTH, y / TILEHEIGHT), x % TILEWIDTH, y % TILEHEIGHT);
}

unsigned int Layer::MaskedHLine(int x, int y, int length) const //negative length not yet supported :(
{
	if (y < 0)
		y = 0;
	else if (y >= HeightPixels)
		y = HeightPixels - 1;
	if (x < 0) {
		if (MaskedPixel(0, y)) return 1;
		if (length += x <= 0)
			return 0;
		x = 0;
	}
	const int xPosInTile = x % TILEWIDTH;
	const int yPosInTile = y % TILEHEIGHT;
	x /= TILEWIDTH;
	y /= TILEHEIGHT;
	unsigned int lengthElapsed = 0;
	if (xPosInTile != 0) {
		const unsigned int lengthInTile = min(length, TILEWIDTH - xPosInTile);
		const unsigned int firstTileResult = LevelPtr->TilesetPtr->MaskedHLine(GetTile(x, y), xPosInTile, yPosInTile, lengthInTile);
		if (firstTileResult != 0)
			return firstTileResult;
		length -= lengthInTile;
		lengthElapsed += lengthInTile;
		++x;
	}
	while (length > 0) {
		if (x >= Width)
			return 0;
		const unsigned int lengthInTile = min(length, TILEWIDTH);
		const unsigned int laterTileResult = LevelPtr->TilesetPtr->MaskedHLine(GetTile(x, y), 0, yPosInTile, lengthInTile);
		if (laterTileResult != 0)
			return laterTileResult + lengthElapsed;
		length -= lengthInTile;
		lengthElapsed += lengthInTile;
		++x;
	}
	return 0;
}

unsigned int Layer::MaskedVLine(int x, int y, int length) const //fractionally edited from MaskedHLine--make changes above to here below too
{
	if (x < 0)
		x = 0;
	else if (x >= WidthPixels)
		x = WidthPixels - 1;
	if (y < 0) {
		if (MaskedPixel(x, 0)) return 1;
		if (length += y <= 0)
			return 0;
		y = 0;
	}
	const int xPosInTile = x % TILEWIDTH;
	const int yPosInTile = y % TILEHEIGHT;
	x /= TILEWIDTH;
	y /= TILEHEIGHT;
	unsigned int lengthElapsed = 0;
	if (yPosInTile != 0) {
		const unsigned int lengthInTile = min(length, (TILEHEIGHT - 1) - yPosInTile);
		const unsigned int firstTileResult = LevelPtr->TilesetPtr->MaskedVLine(GetTile(x, y), xPosInTile, yPosInTile, lengthInTile);
		if (firstTileResult != 0)
			return firstTileResult;
		length -= lengthInTile;
		lengthElapsed += lengthInTile;
		++y;
	}
	while (length > 0) {
		if (y >= Height)
			return 0;
		const unsigned int lengthInTile = min(length, TILEHEIGHT);
		const unsigned int laterTileResult = LevelPtr->TilesetPtr->MaskedVLine(GetTile(x, y), xPosInTile, 0, lengthInTile);
		if (laterTileResult != 0)
			return laterTileResult + lengthElapsed;
		length -= lengthInTile;
		lengthElapsed += lengthInTile;
		++y;
	}
	return 0;
}

void Layer::Update(unsigned int gameTicks, unsigned int animOffset, sf::Vector2f camera)
{
	if (!IsUsed)
		return;
	
	if (!AutoSpeedX || IsTextured)
		PositionX = (camera.x + int(WINDOW_WIDTH_PIXELS - 320) / 2.f) * SpeedX; //320 and 200 are the numbers JJ2 happened to use for this purpose
	else
		PositionX = gameTicks * AutoSpeedX;
	if (!AutoSpeedY || IsTextured)
		PositionY = (camera.y + int(WINDOW_HEIGHT_PIXELS - 200) / 2.f) * SpeedY;
	else
		PositionY = gameTicks * AutoSpeedY;
	PositionX += int(320 - WINDOW_WIDTH_PIXELS) / 2.f;
	if (!LimitVisibleRegion || IsHeightTiled)
		PositionY += int(200 - WINDOW_HEIGHT_PIXELS) / 2.f;
	else
		PositionY += int(200 - WINDOW_HEIGHT_PIXELS);
	setPosition(roundf(-PositionX), roundf(-PositionY)); //sub-pixel repositioning leads to tearing

	if (ForceUpdate || abs(PositionX - LastPositionX) >= EXTRA_TILES_TO_RENDER_TO_VERTEX_ARRAY * TILEWIDTH || abs(PositionY - LastPositionY) >= EXTRA_TILES_TO_RENDER_TO_VERTEX_ARRAY * TILEHEIGHT) {
		ForceUpdate = false;
		LastPositionX = PositionX;
		LastPositionY = PositionY;
		for (int i = 0; i < NUMBEROFTILESETTEXTURES; ++i)
			Vertices[i].resize(0);
		int startTileX = int(PositionX / TILEWIDTH);
		int startTileY = int(PositionY / TILEWIDTH);
		for (int x = -EXTRA_TILES_TO_RENDER_TO_VERTEX_ARRAY; x < (WINDOW_WIDTH_TILES + EXTRA_TILES_TO_RENDER_TO_VERTEX_ARRAY + 1); ++x)
			for (int y = -EXTRA_TILES_TO_RENDER_TO_VERTEX_ARRAY; y < (WINDOW_HEIGHT_TILES + EXTRA_TILES_TO_RENDER_TO_VERTEX_ARRAY + 1); ++y) {
				int tileX = startTileX + x, tileY = startTileY + y;
				if (IsWidthTiled) { //this stuff can probably be optimized a bit by checking at some higher level?
					if (tileX >= RealWidth)
						tileX %= RealWidth;
					else while (tileX < 0)
						tileX += RealWidth;
				}
				else if (tileX < 0 || tileX >= RealWidth)
					continue;
				if (IsHeightTiled) {
					if (tileY >= Height)
						tileY %= Height;
					else while (tileY < 0)
						tileY += Height;
				}
				else if (tileY < 0 || tileY >= Height)
					continue;
				const Tile tile = GetTile(tileX, tileY);
				if (tile.ID == 0)
					continue;
				if (tile.ID >= animOffset)
					ForceUpdate = true;
				quad tileQuad(LevelPtr->QuadsPerTile, tile);
				tileQuad.positionPositionAt((startTileX + x) * TILEWIDTH, (startTileY + y) * TILEHEIGHT);
				tileQuad.appendTo(Vertices[tileQuad.TextureID]);
			}
	}
}

void Layer::ClearSpriteQueue()
{
	SpriteQueue.resize(0);
}

void Layer::DrawQuad(quad& q, sf::Texture* texture)
{
	if (SpriteQueue.empty() || !SpriteQueue.back().Matches(texture))
		SpriteQueue.push_back(VertexCollection(texture));
	SpriteQueue.back().AppendQuad(q);
}

void Layer::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	if (!IsUsed)
		return;
	states.transform = getTransform();
	for (int i = 0; i < NUMBEROFTILESETTEXTURES; ++i) if (!Vertices[i].empty()) {
		states.texture = LevelPtr->TilesetPtr->TileImages[i];
		target.draw(Vertices[i].data(), Vertices[i].size(), sf::Quads, states);
	}
	for (std::vector<VertexCollection>::const_iterator it = SpriteQueue.begin(); it != SpriteQueue.end(); ++it)
		target.draw(*it, states);
}
