#pragma once
#include <string>
#include "SFML/Config.hpp"
#include "SFML/Graphics.hpp"
#include "Files.h"
#include "Tile.h"
#include "Constants.h"

class Tileset : public LevelTileset {
	const static sf::Uint32 ProperHeaderSignature = 0xAFBEADDE;
private:
	struct TilesetHeader {
	//private:
		char Copyright[FILE_HEADER_COPYRIGHTLENGTH];
		char Magic[FILE_HEADER_MAGICLENGTH];// "TILE"
		sf::Uint8 Signature[4];// 0xAFBEADDE
		char TilesetName[FILE_NAMEFIELDLENGTH];

		bool Matches();
	public:
		FileVersion Version;

		std::wstring GetTilesetName();
		bool Read(std::ifstream&);
	};
	static_assert(sizeof(TilesetHeader) == 222, "TilesetHeader incorrect size!");

	sf::Texture Palette;
	unsigned int TileCount;
	const sf::Uint8* TileMaskPtrs[MAX_TILES * 2];

	unsigned int SpecificFileTypeHeaderSize() override;
	bool ReadSpecificFileHeader(std::ifstream&) override;
	Tileset(std::wstring&);
public:
	sf::Texture* TileImages[NUMBEROFTILESETTEXTURES];

	static Tileset* LoadTileset(std::wstring& Filepath, const char*);
	bool ProcessTilesetData(const char*);

	const sf::Uint8* GetTileMask(Tile) const;
	bool MaskedPixel(Tile, unsigned int, unsigned int) const;
	unsigned int MaskedVLine(Tile, unsigned int, unsigned int, unsigned int) const;
	unsigned int MaskedHLine(Tile, unsigned int, unsigned int, unsigned int) const;
};
