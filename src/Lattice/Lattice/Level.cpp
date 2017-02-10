#include <sys/timeb.h>
#include "Windows.h"
#include "Level.h"
#include "Tileset.h"
#include "Misc.h"
#include "Resources.h"
#include "Lattice.h"
#include "LatticeHooks.h"

std::wstring Level::LevelHeader::GetLevelName()
{
	return WStringFromCharArray(LevelName);
}

bool Level::LevelHeader::Matches()
{
	if (memcmp(Copyright, Jazz2FileCopyrightString, FILE_HEADER_COPYRIGHTLENGTH)) {
		ShowErrorMessage(L"Invalid header (copyright string doesn't match)");
		return false;
	}
	if (memcmp(Magic, "LEVL", FILE_HEADER_MAGICLENGTH)) {
		ShowErrorMessage(L"Invalid header (magic string doesn't match)");
		return false;
	}
	if (!(Version == 0x202 || Version == 0x203)) {
		ShowErrorMessage(L"Invalid header (file version not recognized)");
		return false;
	}
	return true;
}
bool Level::LevelHeader::Read(std::ifstream& file)
{
	file.read((char*)this, sizeof(LevelHeader));
	if (!Matches())
		return false;
	return true;
}


unsigned int Level::SpecificFileTypeHeaderSize()
{
	return sizeof(LevelHeader);
}

bool Level::ReadSpecificFileHeader(std::ifstream& File) {
	LevelHeader headerPart1;
	if (!headerPart1.Read(File))  //includes its own debug messages
		return false;
	Name = headerPart1.GetLevelName();
	Version = headerPart1.Version;
	//OutputDebugString(Name.c_str());
	return true;
}
Level::Level(std::wstring& fn) : LevelTileset(fn), TilesetPtr(nullptr), GameTicks(0) {}
void Level::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
	for (int layerID = LEVEL_LAYERCOUNT - 1; layerID >= 0; --layerID)
		target.draw(Layers[layerID], states);
}
Event& Level::GetEvent(unsigned int x, unsigned int y)
{
	return ((Event*)UncompressedData[1].data())[x + y * WidthTiles];
}
void Level::ForEachEvent(std::function<void(Event&, int, int)> func)
{
	Event* eventPtr = (Event*)UncompressedData[1].data();
	const int area = WidthTiles * HeightTiles;
	for (int i = 0; i < area; ++i) {
		Event& ev = *eventPtr++;
		if (ev.ID)
			func(ev, i % WidthTiles, i / WidthTiles);
	}
}
Level* Level::LoadLevel(std::wstring& Filepath)
{
	if (!(Filepath.length() > 4 && Filepath.substr(Filepath.length() - 4, 4) == L".j2l"))
		Filepath += L".j2l";
	Level* newLevel = new Level(Filepath);
	if (newLevel->Open() && newLevel->ProcessLevelData()) {
		Lattice::SetWindowTitle(newLevel->Name);
		return newLevel;
	} else {
		ShowErrorMessage((Filepath + L" encountered an error").c_str());
		delete newLevel;
		return nullptr;
	}
}

bool Level::ProcessLevelData() //called after Open()
{
	const char* data1Ptr = (const char*)&UncompressedData[0][11]; //skip over a bunch of useless values
	Word* data3Ptr = (Word*)UncompressedData[2].data();
	const WordID* data4Ptr = (const WordID*)UncompressedData[3].data();

	AnimCount = *(sf::Uint16*)data1Ptr;
	AnimOffset = MAX_TILES - AnimCount;

	const bool needToUpdateRawTileValues = Version <= 0x202; //1.23-style level
	const int numberOfTilesInData1Arrays = (Version <= 0x202) ? 1024 : MAX_TILES;
	if (needToUpdateRawTileValues) {
		Word* word = data3Ptr;
		const sf::Uint32 wordCount = GenericFileHeader.DataSizes[2].UData / sizeof(Word);
		for (sf::Uint32 i = 0; i < wordCount; ++i)
			(word++)->UpdateLevelVersion(AnimOffset - 0xC00);
		//Version = 0x203; //hmmmm
	}

	data1Ptr += 8; //level name
	if (WStringFromCharArray(data1Ptr) != Name) {
		ShowErrorMessage(L"Invalid internal levelname");
		return false;
	}

	data1Ptr += FILE_NAMEFIELDLENGTH;
	std::wstring tilesetFilename = WStringFromCharArray(data1Ptr);
	data1Ptr += FILE_NAMEFIELDLENGTH * 2; //skip over BonusLevel
	NextLevel = WStringFromCharArray(data1Ptr);
	data1Ptr += FILE_NAMEFIELDLENGTH * 2; //skip over SecretLevel
	Music = WStringFromCharArray(data1Ptr);
	data1Ptr += FILE_NAMEFIELDLENGTH + LEVEL_HELPSTRINGCOUNT * LEVEL_HELPSTRINGLENGTH;

	for (int layerID = 0; layerID < LEVEL_LAYERCOUNT; ++layerID) {
		new(&Layers[layerID]) Layer(this, data1Ptr, data3Ptr, data4Ptr, layerID);
		//OutputDebugStringF(L"%d: %d at 0,4", layerID, Layers[layerID].GetTile(0, 4));
	}
	WidthTiles = Layers[SPRITELAYER].Width; WidthPixelsF = float(WidthPixels = WidthTiles * TILEWIDTH);
	HeightTiles = Layers[SPRITELAYER].Height; HeightPixelsF = float(HeightPixels = HeightTiles * TILEHEIGHT);

	data1Ptr += 51 * LEVEL_LAYERCOUNT; //all the various layer properties
	data1Ptr += 2; //animoffset, but version-dependent so I derive it above instead
	data1Ptr += 5 * numberOfTilesInData1Arrays; //TilesetEvents + IsEachTileFlipped;
	memcpy(TileTypes, data1Ptr, numberOfTilesInData1Arrays);
	const size_t folderCutoffPoint = Filepath.find_last_of(L"\\");
	if (folderCutoffPoint < Filepath.length())
		tilesetFilename = Filepath.substr(0, folderCutoffPoint) + L"\\" + tilesetFilename;
	TilesetPtr = Tileset::LoadTileset(tilesetFilename, TileTypes);
	if (TilesetPtr == nullptr)
		return false;

	for (int i = 0; i < MAX_TILES; ++i) {
		QuadsPerTile[i].TextureID = i / TILESPERTEXTURE;
		QuadsPerTile[i].useTexCoordsOfTile(i);
	}

	data1Ptr += 2 * numberOfTilesInData1Arrays; //skip over XMask
	AnimatedTiles.resize(AnimCount);
	memcpy(AnimatedTiles.data(), data1Ptr, AnimCount * sizeof(AnimatedTile));
	for (auto& it : AnimatedTiles) {
		if (needToUpdateRawTileValues)
			it.UpdateLevelVersion(AnimOffset - 0xC00);
		it.GenerateFullFrameList(AnimatedTileFrames);
	}
	int animTileID = AnimOffset;
	for (const auto& it : AnimatedTiles) {
		QuadsPerTile[animTileID] = QuadsPerTile[it.CurFrame.ID];
		TilesetPtr->CopyMask(animTileID, it.CurFrame.ID);
		++animTileID;
	}

	//more bytes may be stored after this in the .j2l depending on which level editor was used, but they're all useless

	UncompressedData[0].resize(0); UncompressedData[0].shrink_to_fit(); //this section is no longer needed

	PreloadedAnimationsList defaultAnimList;
	Hook_GetAnimationList(*this, defaultAnimList);

	ForEachEvent([&defaultAnimList](Event& ev, int, int) { //find additional animations to load depending on which objects (enemies, etc.) are in the level
		if (Lattice::ObjectInitializationList->count(ev.ID))
			defaultAnimList.insert(Lattice::ObjectInitializationList->at(ev.ID).AnimSetID);
	});
	AnimFile::ReadAnims(std::wstring(L"Anims.j2a"), defaultAnimList, spriteManager);

	Hook_LevelLoad(*this);

	return true;
}

Tile Level::GetRealTile(Tile inputTile) const
{
	if (inputTile.ID >= AnimOffset) {
		Tile realTile = GetRealTile(AnimatedTiles[inputTile.ID - AnimOffset].CurFrame);
		realTile.HFlipped ^= inputTile.HFlipped; //JJ2 does | instead of ^ here, but that behavior isn't really supported by JCS, so I'll say it's a bug
		realTile.VFlipped ^= inputTile.VFlipped;
		realTile.Rotated ^= inputTile.Rotated; //this isn't quite so commutative.. hmmm
		return realTile;
	}
	return inputTile;
}

void Level::UpdateAnimatedTiles() {
	int animTileID = AnimOffset;
	const auto time = getCurrentTime();

	for (auto& it : AnimatedTiles) {
		it.Update(time);
		if (it.CurFrame.ID != it.LastCurFrame) {
			QuadsPerTile[animTileID] = QuadsPerTile[it.CurFrame.ID];
			TilesetPtr->CopyMask(animTileID, it.CurFrame.ID);
		}
		++animTileID;
	}
}
void Level::Update(const KeyStates& keys)
{
	++GameTicks;

	UpdateAnimatedTiles(); //or do I want this in draw instead?

	for (int layerID = LEVEL_LAYERCOUNT - 1; layerID >= 0; --layerID)
		Layers[layerID].Clear();
	Hook_ClearSpriteQueues();

	Hook_ActivateObjects(*this);
	GameState gameState(*this, keys);

	Objects.remove_if([](auto& p) { return p->Active == false; });
	//for (auto& it : ObjectsToAdd)
	//	Objects.emplace_back(it);
	//ObjectsToAdd.clear();
	for (auto& it : Objects) {
 		it->Draw(Layers);
		it->Behave(gameState);
	}
	for (auto& it : Objects)
		for (auto& it2 : Objects)
			if (Hook_CollideObjects(*it, *it2) && it->CollidesWith(*it2)) {
				it2->HitBy(*it);
			}

	for (int layerID = LEVEL_LAYERCOUNT - 1; layerID >= 0; --layerID)
		Layers[layerID].Update(GameTicks, Camera);

	Hook_LevelMain(*this, GameTicks);
}

bool GameState::MaskedPixel(int x, int y) const {
	return Lev.Layers[SPRITELAYER].MaskedPixel(x, y);
}
unsigned int GameState::MaskedHLine(int x, int y, int length) const {
	const auto retval = Lev.Layers[SPRITELAYER].MaskedHLine(x, y, length);
	//AnimFrame::AppendRectangle(Lev.Layers[SPRITELAYER], x, y, length, 1, retval ? 16 : 24);
	return retval;
}
unsigned int GameState::MaskedVLine(int x, int y, int length) const {
	const auto retval = Lev.Layers[SPRITELAYER].MaskedVLine(x, y, length);
	//AnimFrame::AppendRectangle(Lev.Layers[SPRITELAYER], x, y, 1, length, retval ? 16 : 24);
	return retval;
}
bool GameState::MaskedPixel(int x, int y, Event& outEvent) const {
	const auto retval = MaskedPixel(x, y);
	if (retval) {
		outEvent = Lev.GetEvent(x / TILEWIDTH, y / TILEHEIGHT);
		return true;
	}
	outEvent = 0;
	return false;
}
unsigned int GameState::MaskedHLine(int x, int y, int length, Event& outEvent) const {
	const auto retval = MaskedHLine(x, y, length);
	if (retval) {
		outEvent = Lev.GetEvent((x + retval-1) / TILEWIDTH, y / TILEHEIGHT);
		return retval;
	}
	outEvent = 0;
	return 0;
}
unsigned int GameState::MaskedVLine(int x, int y, int length, Event& outEvent) const {
	const auto retval = MaskedVLine(x, y, length);
	if (retval) {
		outEvent = Lev.GetEvent(x / TILEWIDTH, (y + retval - 1) / TILEHEIGHT);
		return retval;
	}
	outEvent = 0;
	return 0;
}

Event& GameState::GetEvent(unsigned int x, unsigned int y) const
{
	return Lev.GetEvent(x, y);
}

void GameState::SetCamera(float x, float y)
{
	Lev.Camera.x = max(0, min(Lev.WidthPixelsF - WINDOW_WIDTH_PIXELS, x));
	Lev.Camera.y = max(0, min(Lev.HeightPixelsF- WINDOW_HEIGHT_PIXELS, y));
}

void GameState::CenterCamera(float x, float y)
{
	SetCamera(x - WINDOW_WIDTH_PIXELS / 2, y - WINDOW_HEIGHT_PIXELS / 2);
}

void Stage::MakeNewStage() {
	Lattice::Stages.emplace(this);
}
void Stage::DeleteCurrentStage() {
	if (!Lattice::Stages.empty()) {
		Lattice::StageToDelete.swap(Lattice::Stages.top());
		Lattice::Stages.pop();
	}
}
void Stage::ReplaceWithNewStage(Stage* stage) {
	if (!Lattice::Stages.empty() && Lattice::Stages.top().get() == this) {
		DeleteCurrentStage(); //delete me
		stage->MakeNewStage();
	}
}

const AnimSet& Stage::GetAnimSet(int setID) const {
	return spriteManager.AnimationSets[setID];
}
sf::Sound & Stage::StartSound(unsigned int setID, unsigned int sampleID) const {
	return GetAnimSet(setID).StartSound(sampleID);
}
sf::Sound & Stage::StartSound(unsigned int setID, unsigned int sampleID, sf::Vector2f Position, unsigned int volume, unsigned int frequency) const {
	return GetAnimSet(setID).StartSound(sampleID, Position, volume, frequency);
}
