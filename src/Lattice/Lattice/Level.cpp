#include <sys/timeb.h>
#include "Windows.h"
#include "Level.h"
#include "Tileset.h"
#include "Misc.h"
#include "Resources.h"
#include "Game.h"

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
	return ((Event*)UncompressedData[1].data())[x + y * HeightTiles];
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
Level* Level::LoadLevel(std::wstring& Filepath, PreloadedAnimationsList& anims, ObjectList& objs)
{
	Level* newLevel = new Level(Filepath);
	if (newLevel->Open() && newLevel->ProcessLevelData(anims, objs)) {
		return newLevel;
	} else {
		ShowErrorMessage((Filepath + L" encountered an error").c_str());
		delete newLevel;
		return nullptr;
	}
}

bool Level::ProcessLevelData(PreloadedAnimationsList defaultAnimList, ObjectList& objectInitializationList) //called after Open()
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
	for (std::vector<AnimatedTile>::iterator it = AnimatedTiles.begin(); it != AnimatedTiles.end(); ++it) {
		if (needToUpdateRawTileValues)
			it->UpdateLevelVersion(AnimOffset - 0xC00);
		it->GenerateFullFrameList(AnimatedTileFrames);
	}
	int animTileID = AnimOffset;
	for (std::vector<AnimatedTile>::iterator it = AnimatedTiles.begin(); it != AnimatedTiles.end(); ++it, ++animTileID)
		QuadsPerTile[animTileID] = QuadsPerTile[it->CurFrame.ID];

	//more bytes may be stored after this in the .j2l depending on which level editor was used, but they're all useless

	UncompressedData[0].resize(0); UncompressedData[0].shrink_to_fit(); //this section is no longer needed

	ForEachEvent([&defaultAnimList, &objectInitializationList](Event& ev, int, int) { //find additional animations to load depending on which objects (enemies, etc.) are in the level
		if (objectInitializationList.count(ev.ID))
			defaultAnimList.insert(objectInitializationList[ev.ID].AnimSetID);
	});
	AnimFile::ReadAnims(std::wstring(L"Anims.j2a"), defaultAnimList);

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

	for (std::vector<AnimatedTile>::iterator it = AnimatedTiles.begin(); it != AnimatedTiles.end(); ++it, ++animTileID) {
		it->Update(time);
		if (it->CurFrame.ID != it->LastCurFrame)
			QuadsPerTile[animTileID] = QuadsPerTile[it->CurFrame.ID];
	}
}
void Level::Update(sf::Vector2i mousePosition, ObjectActivityFunction& updateActiveObjects)
{
	++GameTicks;

	UpdateAnimatedTiles(); //or do I want this in draw instead?

	for (int layerID = LEVEL_LAYERCOUNT - 1; layerID >= 0; --layerID)
		Layers[layerID].ClearSpriteQueue();

	updateActiveObjects(*this);
	for (std::deque<std::unique_ptr<GameObject>>::iterator it = Objects.begin(); it != Objects.end(); ++it) {
		(**it).Behave(*this);
		//if ((**it).IsActive)
			(**it).Draw(Layers);
	}

	const sf::Vector2f camera(
		mousePosition.x / float(WINDOW_WIDTH_PIXELS) * (WidthPixelsF - WINDOW_WIDTH_PIXELS),
		mousePosition.y / float(WINDOW_HEIGHT_PIXELS) * (HeightPixelsF - WINDOW_HEIGHT_PIXELS)
	);
	for (int layerID = LEVEL_LAYERCOUNT - 1; layerID >= 0; --layerID)
		Layers[layerID].Update(GameTicks, AnimOffset, camera);
}

bool Level::MaskedPixel(int x, int y) const {
	return Layers[SPRITELAYER].MaskedPixel(x, y);
}
unsigned int Level::MaskedHLine(int x, int y, int length) const {
	return Layers[SPRITELAYER].MaskedHLine(x, y, length);
}
unsigned int Level::MaskedVLine(int x, int y, int length) const {
	return Layers[SPRITELAYER].MaskedHLine(x, y, length);
}