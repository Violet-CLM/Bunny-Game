#pragma once
#include <vector>
#include "SFML/Config.hpp"
#include "Constants.h"

struct Tile {
	TileID raw;

	Tile();
	Tile(const Tile& in);
	Tile(TileID, bool = false, bool = false, bool = false);

	TileID GetID() const;
	TileID SetID(TileID n);
	bool GetHFlipped() const;
	bool SetHFlipped(bool f);
	bool GetVFlipped() const;
	bool SetVFlipped(bool f);
	bool GetRotated() const;
	bool SetRotated(bool f);
	__declspec(property(get = GetID, put = SetID)) TileID ID;
	__declspec(property(get = GetHFlipped, put = SetHFlipped)) bool HFlipped;
	__declspec(property(get = GetVFlipped, put = SetVFlipped)) bool VFlipped;
	__declspec(property(get = GetRotated, put = SetRotated)) bool Rotated;

	void UpdateLevelVersion(sf::Uint16);

	void operator=(const Tile& in) {
		raw = in.raw;
	}
//	void operator=(const TileID& in) {
//		raw = in;
//	}
	bool operator==(const Tile& in) const {
		return raw == in.raw;
	}
//	bool operator==(const TileID& in) const {
//		return raw == in;
//	}
};
static_assert(sizeof(Tile) == sizeof(TileID), "Tile incorrect size!");

struct Word { //need an assignable type to create a vector from, which C++ arrays don't qualify as
private:
	union {
		Tile tiles[TILES_IN_WORD];
		long long raw;
	};
public:
	Word() {
		raw = 0;
	}
	void operator=(const Word& in) {
		raw = in.raw;
	}
	bool operator==(const Word& in) const {
		return raw == in.raw;
	}
	Word(const Word& in) {
		this->operator=(in);
	}
	Tile& operator[] (int x) {
		return tiles[x];
	}
	const Tile& operator[] (int x) const {
		return tiles[x];
	}
	void UpdateLevelVersion(sf::Uint16 AnimOffset) {
		for (int i = 0; i < TILES_IN_WORD; ++i)
			tiles[i].UpdateLevelVersion(AnimOffset);
	}
};
static_assert(sizeof(Word) == sizeof(Tile)*TILES_IN_WORD, "Word incorrect size!");

#define AnimatedTilePropertyT(name, type, offset) \
	type Get ## name() const {\
		return *(type *)&(PoorlyAlignedData[offset]);\
	}\
	void Set ## name(type n) {\
		*(type *)&(PoorlyAlignedData[offset]) = n;\
	}\
	__declspec(property(get = Get ## name, put = Set ## name)) type name;
#define AnimatedTileProperty(name, size, offset) AnimatedTilePropertyT(name, sf::Uint ## size, offset)

struct AnimatedTile {
private:
	char PoorlyAlignedData[137];

	//for loading
	AnimatedTileProperty(FrameWait, 16, 0);
	AnimatedTileProperty(RandomWait, 16, 2);
	AnimatedTileProperty(PingPongWait, 16, 4);
	AnimatedTileProperty(PingPong, 8, 6);
public:
	AnimatedTileProperty(Speed, 8, 7);
private:
	AnimatedTileProperty(InitialFrameCount, 8, 8);
	Tile& operator[] (int);
	const Tile& operator[] (int) const;
	//for drawing
	AnimatedTileProperty(TotalFrameCount, 32, 9);
	AnimatedTileProperty(FirstFrame, 32, 13)
	AnimatedTileProperty(FrameID, 32, 17);
	AnimatedTilePropertyT(FrameList, std::vector<Tile>*, 21);

public:
	void UpdateLevelVersion(sf::Uint16);
	void GenerateFullFrameList(std::vector<Tile>&);
	void Update(double);

	AnimatedTilePropertyT(CurFrame, Tile, 25);
	AnimatedTilePropertyT(LastCurFrame, TileID, 27);
};
static_assert(sizeof(AnimatedTile) == 137, "AnimatedTile incorrect size!");