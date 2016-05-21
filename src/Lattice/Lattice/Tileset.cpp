#include "Windows.h"
#include "Tileset.h"
#include "Drawing.h"
#include "Misc.h"

std::wstring Tileset::TilesetHeader::GetTilesetName()
{
	return WStringFromCharArray(TilesetName);
}
bool Tileset::TilesetHeader::Matches()
{
	if (memcmp(Copyright, Jazz2FileCopyrightString, FILE_HEADER_COPYRIGHTLENGTH)) {
		ShowErrorMessage(L"Invalid header (copyright string doesn't match)");
		return false;
	}
	if (memcmp(Magic, "TILE", FILE_HEADER_MAGICLENGTH)) {
		ShowErrorMessage(L"Invalid header (magic string doesn't match)");
		return false;
	}
	if (memcmp(Signature, &ProperHeaderSignature, sizeof(sf::Uint32))) {
		ShowErrorMessage(L"Invalid header (signature doesn't match)");
		return false;
	}
	if (!(Version == 0x200 || Version == 0x201)) {
		ShowErrorMessage(L"Invalid header (file version not recognized)");
		return false;
	}
	return true;
}
bool Tileset::TilesetHeader::Read(std::ifstream& file)
{
	file.read((char*)this, sizeof(TilesetHeader));
	if (!Matches())
		return false;
	return true;
}


unsigned int Tileset::SpecificFileTypeHeaderSize()
{
	return sizeof(TilesetHeader);
}

bool Tileset::ReadSpecificFileHeader(std::ifstream& File) {
	TilesetHeader headerPart1;
	if (!headerPart1.Read(File))  //includes its own debug messages
		return false;
	Name = headerPart1.GetTilesetName();
	Version = headerPart1.Version;
	//OutputDebugString(Name.c_str());
	return true;
}
Tileset::Tileset(std::wstring& fn) : LevelTileset(fn) {}
Tileset* Tileset::LoadTileset(std::wstring& Filepath, const char* TileTypes)
{
	Tileset* newTileset = new Tileset(Filepath);
	if (newTileset->Open() && newTileset->ProcessTilesetData(TileTypes)) {
		return newTileset;
	} else {
		ShowErrorMessage((Filepath + L" encountered an error").c_str());
		delete newTileset;
		return nullptr;
	}
}

bool Tileset::ProcessTilesetData(const char* TileTypes) //called after Open()
{
	const sf::Uint8* data1Ptr = UncompressedData[0].data();
	GeneratePaletteTexture(Palette, data1Ptr);
	data1Ptr += COLORSPERPALETTE * BYTESPER32BITPIXEL;
	TileCount = *(sf::Uint32*)data1Ptr;
	data1Ptr += sizeof(sf::Uint32);

	const int numberOfTilesInData1Arrays = (Version <= 0x200) ? 1024 : MAX_TILES;
	data1Ptr += numberOfTilesInData1Arrays * 2; //FullyOpaque, useless
	GenerateTilesetTextures(TileImages, TileTypes, (const sf::Uint32*)data1Ptr, UncompressedData[1].data(), TileCount);
	data1Ptr += numberOfTilesInData1Arrays * 4 * 4; //skip over TMaskAddress

	const sf::Uint32* MaskAddress = (const sf::Uint32*)data1Ptr;
	const sf::Uint8* Masks = UncompressedData[3].data();
	for (int i = 0; i < numberOfTilesInData1Arrays; ++i)
		TileMaskPtrs[i] = *MaskAddress++ + Masks;
	for (int i = 0; i < numberOfTilesInData1Arrays; ++i)
		TileMaskPtrs[MAX_TILES + i] = *MaskAddress++ + Masks;

	for (int i = 0; i < 3; ++i) { //these sections are no longer needed
		UncompressedData[i].resize(0); UncompressedData[i].shrink_to_fit();
	}
	return true;
}

const sf::Uint8* Tileset::GetTileMask(Tile tile) const
{
	return TileMaskPtrs[tile.ID + int(tile.HFlipped) * MAX_TILES];
}

void Tileset::CopyMask(int destID, int srcID)
{
	TileMaskPtrs[destID] = TileMaskPtrs[srcID];
	TileMaskPtrs[destID ^ TILE_HFLIPPED] = TileMaskPtrs[srcID ^ TILE_HFLIPPED];
}

bool Tileset::MaskedPixel(Tile tile, unsigned int x, unsigned int y) const
{
	if (!tile.ID)
		return false;
	if (tile.VFlipped)
		y = TILEHEIGHT - 1 - y;
	if (tile.Rotated) {
		unsigned int swap = x;
		x = TILEWIDTH - 1 - y;
		y = swap;
	}
	return !!(GetTileMask(tile)[(x + y*TILEWIDTH) / BITS_PER_MASKBYTE] & (1 << (x % BITS_PER_MASKBYTE)));
}

unsigned int Tileset::MaskedVLine(Tile tile, unsigned int x, unsigned int y, unsigned int length) const
{
	if (!tile.ID)
		return 0;
	if (tile.VFlipped)
		y = TILEHEIGHT - 1 - y;
	if (tile.Rotated) {
		tile.Rotated = false;
		//todo
	}
	const sf::Uint8* mask = GetTileMask(tile) + (x + y*TILEWIDTH) / BITS_PER_MASKBYTE;
	const int maskRowAdjustment = (!tile.VFlipped) ? (TILEWIDTH / BITS_PER_MASKBYTE) : (-TILEWIDTH / BITS_PER_MASKBYTE);
	const unsigned int xPositionInByteMask = 1 << (x % BITS_PER_MASKBYTE);
	for (unsigned int distanceTested = 0; distanceTested < length; ++distanceTested, mask += maskRowAdjustment)
		if (!!(*mask & xPositionInByteMask))
			return distanceTested + 1;
	return 0;
}

unsigned int Tileset::MaskedHLine(Tile tile, unsigned int x, unsigned int y, unsigned int length) const
{
	if (!tile.ID)
		return 0;
	if (tile.VFlipped)
		y = TILEHEIGHT - 1 - y;
	if (tile.Rotated) {
		tile.Rotated = false;
		//todo
	}
	const sf::Uint8* mask = GetTileMask(tile) + (x + y*TILEWIDTH) / BITS_PER_MASKBYTE;
	unsigned int distanceTested = 0;
	while (x % BITS_PER_MASKBYTE != 0 && length > distanceTested) {
		++distanceTested;
		if (!!(*mask & (1 << (x % BITS_PER_MASKBYTE))))
			return distanceTested;
		++x;
	}
	++mask;
	while (int(length - distanceTested) >= BITS_PER_MASKBYTE) {
		if (!!*mask)
			for (int i = 0; i < BITS_PER_MASKBYTE; ++i) {
				++distanceTested;
				if (!!(*mask & (1 << i)))
					return distanceTested;
			}
		else distanceTested += BITS_PER_MASKBYTE;
		++mask;
	}
	x = 0;
	while (length > distanceTested) {
		++distanceTested;
		if (!!(*mask & (1 << x)))
			return distanceTested;
		++x;
	}
	return 0;
}
