#pragma once
#include <string>
#include <functional>
#include <forward_list>
#include <memory>
#include "SFML/Config.hpp"
#include "Files.h"
#include "Tileset.h"
#include "Layer.h"
#include "Event.h"
#include "Drawing.h"
#include "Objects.h"

class GameObject;
class KeyStates;
class Level : public LevelTileset, public sf::Drawable {
	friend class GameState;
	friend class GameObject;
private:
	struct LevelHeader {
	private:
		char Copyright[FILE_HEADER_COPYRIGHTLENGTH];
		char Magic[FILE_HEADER_MAGICLENGTH];// "LEVL"
		sf::Uint8 PasswordHash[3]; // 0xBEBA00 for no password
		bool HideLevel;
		char LevelName[FILE_NAMEFIELDLENGTH];

		bool Matches();
	public:
		FileVersion Version;

		std::wstring GetLevelName();
		bool Read(std::ifstream&);
	};
	static_assert(sizeof(LevelHeader) == 222, "LevelHeader incorrect size!");


	sf::Uint16 AnimCount;

	unsigned int WidthTiles;
	unsigned int WidthPixels;
	unsigned int HeightTiles;
	unsigned int HeightPixels;
	float WidthPixelsF;
	float HeightPixelsF;

	char TileTypes[MAX_TILES];
	std::vector<Tile> AnimatedTileFrames;
	std::vector<AnimatedTile> AnimatedTiles;
	void UpdateAnimatedTiles();

	unsigned int SpecificFileTypeHeaderSize() override;
	bool ReadSpecificFileHeader(std::ifstream&) override;
	Level(std::wstring&);

	sf::Vector2f Camera;
	VertexCollectionQueue HUD;
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;

public:
	Tileset* TilesetPtr;
	std::wstring NextLevel;
	std::wstring Music;

	quad QuadsPerTile[MAX_TILES]; //todo wrap this more
	Layer Layers[LEVEL_LAYERCOUNT];
	sf::Uint16 AnimOffset;

	std::forward_list<std::unique_ptr<GameObject>> Objects;
	unsigned int GameTicks;

	static Level* LoadLevel(std::wstring&);
	bool ProcessLevelData();

	Event& GetEvent(unsigned int, unsigned int);
	void ForEachEvent(std::function<void(Event&, int, int)>);
	Tile GetRealTile(Tile) const;

	void Update(const KeyStates&);
};

class GameState {
private:
	Level& Lev;
public:
	const unsigned int& GameTicks;
	const KeyStates& Keys;
	GameState(Level& l, const KeyStates& k) : Lev(l), Keys(k), GameTicks(l.GameTicks) {}

	void SetCamera(float, float);
	void CenterCamera(float, float);

	bool MaskedPixel(int, int) const;				//shortcuts for Layers[SPRITELAYER]
	unsigned int MaskedHLine(int, int, int) const;	//
	unsigned int MaskedVLine(int, int, int) const;	//
	bool MaskedPixel(int, int, Event&) const;		//tileAttr variants
	unsigned int MaskedHLine(int, int, int, Event&) const;	//
	unsigned int MaskedVLine(int, int, int, Event&) const;	//

	Event& GetEvent(unsigned int, unsigned int) const;
};