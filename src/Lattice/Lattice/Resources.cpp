#include <forward_list>
#include "Windows.h"
#include "Resources.h"
#include "Drawing.h"
#include "Misc.h"

extern "C" { FILE __iob_func[3] = { *stdin,*stdout,*stderr }; } //needed in VS2015 to include flac.lib according to http://stackoverflow.com/questions/30412951/unresolved-external-symbol-imp-fprintf-and-imp-iob-func-sdl2

bool AnimFile::AnimFileHeader::Matches()
{
	if (memcmp(Magic, "ALIB", FILE_HEADER_MAGICLENGTH)) {
		ShowErrorMessage(L"Invalid header (magic string doesn't match)");
		return false;
	}
	if (Signature != ProperHeaderSignature) {
		ShowErrorMessage(L"Invalid header (signature doesn't match)");
		return false;
	}
	if (Version != 0x200 || Version2 != 0x1808) {
		ShowErrorMessage(L"Invalid header (file version not recognized)");
		return false;
	}
	if (SetCount == 0) {
		ShowErrorMessage(L"Invalid header (empty file)");
		return false;
	}
	return true;
}
bool AnimFile::AnimFileHeader::Read(std::ifstream& file)
{
	file.read((char*)this, sizeof(AnimFileHeader));
	if (!Matches())
		return false;
	return true;
}


unsigned int AnimFile::SpecificFileTypeHeaderSize()
{
	return sizeof(AnimFileHeader);
}

bool AnimFile::ReadSpecificFileHeader(std::ifstream& File) {
	AnimFileHeader headerPart1;
	if (!headerPart1.Read(File))  //includes its own debug messages
		return false;
	SetAddresses.resize(headerPart1.SetCount);
	return true;
}
AnimFile::AnimFile(std::wstring& fn, const PreloadedAnimationsList& setIDs, std::vector<AnimSet>& animSets) : AnimationSets(animSets), JazzFile(fn), AnimSetIDs(setIDs) {}
bool AnimFile::ReadAnims(std::wstring& Filepath, const PreloadedAnimationsList& setIDs, SpriteManager& manager)
{
	AnimFile newAnimFile(Filepath, setIDs, manager.AnimationSets);
	if (!newAnimFile.Open()) {
		ShowErrorMessage((Filepath + L" encountered an error").c_str());
		return false;
	}
	
	for (const auto& it : manager.AnimationSets) //find ALL animframes, regardless of their parent animations
		for (const auto& jt : it.Animations)
			for (auto& kt : *jt.AnimFrames)
				manager.AddFrame(kt);
	manager.CreateAndAssignTextures();
	manager.Clear();

	return true;
}

bool AnimFile::ReadStream(std::ifstream& file) {
	if (FileSize <= SpecificFileTypeHeaderSize())
		ShowErrorMessage(L"Invalid header (too small)");
	else if (!ReadSpecificFileHeader(file))
		;//method should include its own debug messages
	else {
		file.read((char*)SetAddresses.data(), sizeof(unsigned int) * SetAddresses.size());
		for (const auto it : AnimSetIDs) {
			//OutputDebugStringF(L"%d", it);
			if (AnimationSets.size() < size_t(1 + it))
				AnimationSets.resize(1 + it);
			file.seekg(SetAddresses[it], std::ios_base::beg);
			AnimationSets[it].LoadFromFile(file);
		}

		return true;
	}

	return false;
}

void AnimFrame::AssignTextureCoordinates(const SpriteCoordinateRectangle* const textureCoordinates) {
	Quad.vertices[0].texCoords = sf::Vector2<float>(float(textureCoordinates->left), float(textureCoordinates->top));
	Quad.vertices[1].texCoords = sf::Vector2<float>(float(textureCoordinates->left + textureCoordinates->width), float(textureCoordinates->top));
	Quad.vertices[2].texCoords = sf::Vector2<float>(float(textureCoordinates->left + textureCoordinates->width), float(textureCoordinates->top + textureCoordinates->height));
	Quad.vertices[3].texCoords = sf::Vector2<float>(float(textureCoordinates->left), float(textureCoordinates->top + textureCoordinates->height));
}

void AnimFrame::AssignTextureDetails(unsigned int t, const SpriteCoordinateRectangle* const textureCoordinates, std::vector<sf::Texture>& SpriteTextures) {
	AssignTextureCoordinates(textureCoordinates);

	(Texture = &SpriteTextures[Quad.TextureID = t])->update(
		(sf::Uint8*)Image.data(),
		textureCoordinates->width,
		textureCoordinates->height,
		textureCoordinates->left,
		textureCoordinates->top
	);
	Image.resize(0); Image.shrink_to_fit();
}


SpriteCoordinateRectangle * SpriteManager::SpriteTreeNode::placeSprite(const unsigned int width, const unsigned int height) {
	//packing algorithm described at http://www.blackpawn.com/texts/lightmaps/default.html, because djazz used it for that wacky sprites-rotating-around-mouse-cursor thing in JS and that means it's proven to work
	if (firstChild != nullptr) {
		SpriteCoordinateRectangle* leftResult = firstChild->placeSprite(width, height);
		if (leftResult != nullptr) return leftResult;
		return secondChild->placeSprite(width, height);
	}
	else {
		if (used)
			return nullptr;

		if (width > rectangle.width || height > rectangle.height)
			return nullptr;

		if (width == rectangle.width && height == rectangle.height)
		{
			used = true;
			return &rectangle;
		}

		//(otherwise, gotta split this node and create some kids)
		firstChild = new SpriteTreeNode;
		secondChild = new SpriteTreeNode;

		//(decide which way to split)
		if ((rectangle.width - width) > (rectangle.height - height)) {
			firstChild->rectangle = SpriteCoordinateRectangle(rectangle.left, rectangle.top,
				width, rectangle.height);
			secondChild->rectangle = SpriteCoordinateRectangle(rectangle.left + width, rectangle.top,
				rectangle.width - width, rectangle.height);
		}
		else {
			firstChild->rectangle = SpriteCoordinateRectangle(rectangle.left, rectangle.top,
				rectangle.width, height);
			secondChild->rectangle = SpriteCoordinateRectangle(rectangle.left, rectangle.top + height,
				rectangle.width, rectangle.height - height);
		}

		return firstChild->placeSprite(width, height);
	}
}

AnimFrame::AnimFrame(const sf::Uint8*& frameInfoData, const sf::Uint8* const imageData)
{
	memcpy((char*)this, frameInfoData, offsetof(AnimFrame, ImageAddress) + 4); //4 instead of sizeof(ImageAddress) in case this is a 64-bit compilation
	frameInfoData += offsetof(AnimFrame, ImageAddress) + 4 * 2; //skip MaskAddress
	ImageAddress += UINT_PTR(imageData);

	const sf::Uint32 alpha = (!(*(ImageAddress + 1) & 0x80u)) ? 0xFF000000u : 0x80000000u; //100% or 50% alpha
	const sf::Uint8* mainSprite = ImageAddress + sizeof(sf::Uint16) * 2; //skip image-internal size//transparency declarations

	Image.resize(Area = Width * Height);
	Quad = quad(float(Width), float(Height));
	sf::Uint32* drawingPixel = Image.data(), *drawingLeftBorder = drawingPixel;
	char spriteDataByte;
	int height = Height;
	do {
		while (true) {
			while (true) {
				spriteDataByte = *mainSprite++;
				if (spriteDataByte < 0)
					break;
				drawingPixel += spriteDataByte;
			}
			spriteDataByte += 128;
			if (!spriteDataByte)
				break;
			do {
				*drawingPixel++ = (alpha | *mainSprite++);
			} while (--spriteDataByte);
		}
		drawingPixel = drawingLeftBorder += Width;
	} while (--height);
}
bool AnimFrame::SmallerThan(unsigned int hardwareMaximumTextureSize) const
{
	return !(Width == 0 || Height == 0 || Width > hardwareMaximumTextureSize || Height > hardwareMaximumTextureSize);
}
void AnimFrame::MovePositionToGunSpotX(float& x, bool xFlipped) const {
	if ( !xFlipped )
		x += HotspotX - GunspotX;
	else
		x += GunspotX - HotspotX;
}
void AnimFrame::MovePositionToGunSpotY(float& y, bool yFlipped) const {
	if ( !yFlipped )
		y += HotspotY - GunspotY;
	else
		y += GunspotY - HotspotY;
}
Animation::Animation(const sf::Uint8*& animInfoData, const sf::Uint8*& frameInfoData, const sf::Uint8* const imageData)
{
	memcpy((char*)this, animInfoData, NumberOfBytesToReadFromFile);
	animInfoData += NumberOfBytesToReadFromFile;

	AnimFrames.reset(new std::vector<AnimFrame>);
	for (int frameID = 0; frameID < FrameCount; ++frameID)
		AnimFrames->emplace_back(frameInfoData, imageData);
}
void AnimSet::LoadFromFile(std::ifstream& file)
{
	char subfileHeader[FILE_HEADER_MAGICLENGTH];
	file.read(subfileHeader, FILE_HEADER_MAGICLENGTH);
	if (memcmp(subfileHeader, "ANIM", FILE_HEADER_MAGICLENGTH))
		ShowErrorMessage(L"Invalid anim subfile (magic string doesn't match)");

	file.read(&AnimationCount, sizeof(AnimationCount));
	file.read(&SampleCount, sizeof(SampleCount));
	sf::Uint16 totalFrameCount;
	file.read((char*)&totalFrameCount, sizeof(totalFrameCount));
	file.seekg(4, std::ios_base::cur); //skip PriorSampleCount

	DataSizes_t DataSizes[FILE_DATASTREAMCOUNT];
	file.read((char*)DataSizes, sizeof(DataSizes));
	if (DataSizes[0].UData != AnimationCount * 8 || DataSizes[1].UData != totalFrameCount * 24)
		ShowErrorMessage(L"Invalid anim subfile (data stream/s of incorrect size)");
	std::array<std::vector<sf::Uint8>, FILE_DATASTREAMCOUNT> uncompressedData;
	if (!JazzFile::Uncompress(file, DataSizes, uncompressedData))
		ShowErrorMessage(L"Invalid anim subfile");

	const sf::Uint8* animInfoData = uncompressedData[0].data();
	const sf::Uint8* frameInfoData = uncompressedData[1].data();
	const sf::Uint8* imageData = uncompressedData[2].data();
	const sf::Uint8* sampleData = uncompressedData[3].data();
	for (int animationID = 0; animationID < AnimationCount; ++animationID)
		Animations.emplace_back(animInfoData, frameInfoData, imageData);
	for (int sampleID = 0; sampleID < SampleCount; ++sampleID)
		LoadSample(sampleData);
}
void AnimSet::LoadSample(const sf::Uint8*& sampleData) { //based on https://www.jazz2online.com/soundboard/thread.js and https://www.jazz2online.com/jcf/showpost.php?p=472384&postcount=191
	unsigned int sfxLength = *(unsigned int*)sampleData; sampleData += sizeof(unsigned int);
	struct {
		sf::Int8 Magic[4];
		sf::Uint32 Len;
		sf::Int8 Format[4];
		sf::Int8 SubChunk[4];
		sf::Uint8 Unknowns1[44];
		sf::Uint16 mul;
		sf::Uint16 Unknown2;
		sf::Uint32 length;
		sf::Uint32 Unknowns3[2];
		sf::Uint32 rate;
		sf::Uint64 Unknown4;
	} RiffInfo;
	static_assert(sizeof(RiffInfo) == 88, "RiffInfo incorrect size!");
	memcpy(&RiffInfo, sampleData, sizeof(RiffInfo)); sampleData += sizeof(RiffInfo);
	//OutputDebugStringF(L"%u", RiffInfo.length);

	if (memcmp(RiffInfo.Magic, "RIFF", 4))
		ShowErrorMessage(L"Sample Data improperly formatted");
		
	auto mul = (RiffInfo.mul & 4) / 4 + 1;
	auto length = RiffInfo.length;
	auto rate = RiffInfo.rate;
	auto time = length / rate;
	length *= mul;

	std::vector<sf::Uint8> waveHeader = {
		'R','I','F','F',
		0,0,0,0,		// 4
		'W','A','V','E',
		'f','m','t',' ',
		16,0,0,0,			// size of the following (16 bytes)
		1,0,				// PCM format
		1,0,				// Mono: 1 channel
		0,0,0,0,		// 24
		0,0,0,0,		// 28
		0,0,0,0,		// 32
		'd','a','t','a',
		0,0,0,0			// 40
	};
	*(sf::Uint32*)(waveHeader.data() + 4) = length+36; //number of bytes remaining in the file after "RIFF" and this four-byte integer: 36 is the length of the remainder of the header (8*4 + 2*2)
	*(sf::Uint32*)(waveHeader.data() +24) = rate;
	*(sf::Uint32*)(waveHeader.data() +28) = rate*mul;
	*(sf::Uint32*)(waveHeader.data() +32) = mul*0x80001;
	*(sf::Uint32*)(waveHeader.data() +40) = length; //size of the following

	mul <<= 7;
	while (length--)
		waveHeader.push_back((*sampleData++ ^ mul));
	sampleData += (RiffInfo.length & 1); //sampledata arrays are padded out to multiples of two, and we need this pointer to be in the right position in case another sample gets loaded after this one

	Samples.emplace_back();
	Samples.back().loadFromMemory(waveHeader.data(), waveHeader.size());
}

std::forward_list<sf::Sound> SoundsPlaying;
sf::Sound& AnimSet::StartSound(unsigned int sampleID) const {
	SoundsPlaying.remove_if([](const auto& s) { return s.getStatus() == sf::SoundSource::Status::Stopped; }); //this seems like a practical enough place to put this
	
	SoundsPlaying.emplace_front(Samples.at(sampleID));
	sf::Sound& sample = SoundsPlaying.front();
	sample.play();

	return sample;
}
sf::Sound& AnimSet::StartSound(unsigned int sampleID, sf::Vector2f Position, unsigned int volume, unsigned int frequency) const {
	sf::Sound& sample = StartSound(sampleID);
	//sample.setPosition( //I have no idea where this should go
	if (volume)
		sample.setVolume(volume * 1.58730159f);
	//if (frequency)
		//sample.setPitch( //look, I don't know
	return sample;
}

void SpriteManager::AddFrame(AnimFrame& newFrame) {
	SpriteTexturesSortedBySize.push_back(&newFrame);
}

void SpriteManager::CreateAndAssignTextures() {
	std::sort(SpriteTexturesSortedBySize.begin(), SpriteTexturesSortedBySize.end(), AnimFrame::SortBySize);

	unsigned int hardwareMaximumTextureSize = sf::Texture::getMaximumSize(); //todo move somewhere else?
	for (auto& it : SpriteTexturesSortedBySize) {
		if (!it->SmallerThan(hardwareMaximumTextureSize))
			continue;

		for (unsigned int textureID = 0; ; ++textureID) {
			while (SpriteTextures.size() <= textureID) { //SpriteTextures and SpriteTrees should always be the same size
				SpriteTextures.emplace_back();
				SpriteTextures.back().create(hardwareMaximumTextureSize, hardwareMaximumTextureSize);

				SpriteTreeNode* newNode = new SpriteTreeNode;
				newNode->rectangle = SpriteCoordinateRectangle(0, 0, hardwareMaximumTextureSize, hardwareMaximumTextureSize);
				SpriteTrees.push_back(newNode);
			}

			if (SpriteTrees[textureID] == nullptr)
				continue;

			SpriteCoordinateRectangle* textureCoordinates = SpriteTrees[textureID]->placeSprite(it->Width, it->Height);
			if (textureCoordinates != nullptr) {
				it->AssignTextureDetails(textureID, textureCoordinates, SpriteTextures);
				break;
			}
		}
	}
}

void SpriteManager::Clear() {
	for (unsigned int textureID = 0; textureID < SpriteTrees.size(); ++textureID)
		if (SpriteTrees[textureID] != nullptr) { //don't worry about whether deleting nullptrs is fully defined
			delete SpriteTrees[textureID]; //deletes its branches recursively
			SpriteTrees[textureID] = nullptr;
		}
	
	//SpriteTextures[0]->copyToImage().saveToFile("C:\\Games\\Jazz2\\SpriteTexture.png");
}
SpriteManager::~SpriteManager() {
	Clear();
}

AnimFrame& SpriteManager::GetFrame(int setID, int animID, int frameID) const {
	return (*AnimationSets[setID].Animations[animID].AnimFrames)[frameID];
}
AnimFrame& SpriteManager::GetFrameLimited(int setID, int animID, int frameID) const {
	auto& animFrames = *AnimationSets[setID].Animations[animID].AnimFrames;
	if (animFrames.size() == 0)
		frameID = 0;
	else
		frameID %= animFrames.size();
	return animFrames[frameID];
}
