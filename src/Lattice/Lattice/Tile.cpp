#include "Windows.h"
#include "Misc.h"
#include "Tile.h"

Tile::Tile() {
	raw = 0;
}

Tile::Tile(const Tile & in) {
	this->operator=(in);
}

Tile::Tile(TileID n, bool h, bool v, bool r)
{
	raw = n | (int(h) << 12) | (int(v) << 13) | (int(r) << 14);
}

TileID Tile::GetID() const {
	return raw & TILE_ID_RANGE;
}

TileID Tile::SetID(TileID n) {
	raw = (n & TILE_ID_RANGE) | (raw & ~TILE_ID_RANGE);
	return n;
}

bool Tile::GetHFlipped() const {
	return !!(raw & TILE_HFLIPPED);
}

bool Tile::SetHFlipped(bool f) {
	if (f) raw |= TILE_HFLIPPED;
	else raw ^= TILE_HFLIPPED;
	return f;
}

bool Tile::GetVFlipped() const {
	return !!(raw & TILE_VFLIPPED);
}

bool Tile::SetVFlipped(bool f) {
	if (f) raw |= TILE_VFLIPPED;
	else raw ^= TILE_VFLIPPED;
	return f;
}

bool Tile::GetRotated() const {
	return !!(raw & TILE_ROTATED);
}

bool Tile::SetRotated(bool f) {
	if (f) raw |= TILE_ROTATED;
	else raw ^= TILE_ROTATED;
	return f;
}

void Tile::UpdateLevelVersion(sf::Uint16 AnimOffset)
{
	if (!!(raw & 0x400)) { //stores HFlipped bool in older .j2l versions
		//OutputDebugStringF(L"%d to %d", raw, ((raw ^ 0x400) | TILE_HFLIPPED));
		raw ^= 0x400;
		HFlipped = true;
	}
	//else OutputDebugStringF(L"No need to update %d!", raw);
	if (ID >= AnimOffset) //animated
		raw += 0xC00;
}

Tile& AnimatedTile::operator[](int n)
{
	return ((Tile*)&(PoorlyAlignedData[9]))[n];
}
const Tile& AnimatedTile::operator[](int n) const
{
	return ((Tile*)&(PoorlyAlignedData[9]))[n];
}

void AnimatedTile::UpdateLevelVersion(sf::Uint16 AnimOffset)
{
	for (int i = 0; i < InitialFrameCount; ++i)
		this->operator[](i).UpdateLevelVersion(AnimOffset);
}

void AnimatedTile::GenerateFullFrameList(std::vector<Tile>& dest)
{
	const unsigned int firstVectorEntry = dest.size();
	unsigned int totalNumberOfFrames = 0;
	for (int cycleCount = (RandomWait != 0) ? 4 : 1; cycleCount; --cycleCount) {
		int frameWait = RandomWait * ((rand() & 0x7FFF) - 0x3FFF) / 0x4000 + FrameWait;
		if (frameWait < 0)
			frameWait = 0;
		if (InitialFrameCount > 0) { //this could be replaced with a memcpy, really
			for (int i = 0; i < InitialFrameCount; ++i)
				dest.push_back(this->operator[](i));
			totalNumberOfFrames += InitialFrameCount;
		}
		if (PingPong) {
			if (PingPongWait) {
				Tile repeatedTile = this->operator[](InitialFrameCount - 1);
				for (int i = 0; i < PingPongWait; ++i)
					dest.push_back(repeatedTile);
				totalNumberOfFrames += PingPongWait;
			}
			if (InitialFrameCount - 1 >= 0) {
				for (int i = InitialFrameCount - 1; i >= 0; --i)
					dest.push_back(this->operator[](i));
				totalNumberOfFrames += InitialFrameCount;
			}
		}
		if (frameWait > 0) {
			Tile repeatedTile = this->operator[](PingPong ? (0) : (InitialFrameCount - 1));
			for (int i = 0; i < frameWait; ++i)
				dest.push_back(repeatedTile);
			totalNumberOfFrames += frameWait;
		}
	}
	//OutputDebugStringF(L"%d", totalNumberOfFrames);
	FrameList = &dest;
	TotalFrameCount = totalNumberOfFrames;
	FirstFrame = firstVectorEntry;
	FrameID = 0;
	CurFrame = (*FrameList)[FirstFrame];
	LastCurFrame = CurFrame.ID;
}

void AnimatedTile::Update(double time)
{
	if (Speed == 0) return;
	LastCurFrame = CurFrame.ID;
	FrameID = sf::Uint32(Speed * time / 1000.0) % TotalFrameCount;
	CurFrame = (*FrameList)[FirstFrame + FrameID];
}
