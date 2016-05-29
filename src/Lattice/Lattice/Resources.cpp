#include "Windows.h"
#include "Resources.h"
#include "Drawing.h"
#include "Misc.h"

std::vector<AnimSet*> AnimationSets;
std::vector<SpriteTreeNode*> SpriteTrees;
std::vector<sf::Texture*> SpriteTextures;

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
AnimFile::AnimFile(std::wstring& fn, PreloadedAnimationsList& setIDs) : JazzFile(fn), AnimSetIDs(setIDs) {}
bool AnimFile::ReadAnims(std::wstring& Filepath, PreloadedAnimationsList& setIDs)
{
	AnimFile newAnimFile(Filepath, setIDs);
	if (!newAnimFile.Open()) {
		ShowErrorMessage((Filepath + L" encountered an error").c_str());
		return false;
	}
	

	std::vector<AnimFrame*> SpriteTexturesSortedBySize;
	for (std::vector<AnimSet*>::const_iterator it = AnimationSets.begin(); it != AnimationSets.end(); ++it) { //find ALL animframes, regardless of their parent animations
		if (*it != nullptr) {
			std::vector<Animation>& animations = (*it)->Animations;
			for (std::vector<Animation>::const_iterator jt = animations.begin(); jt != animations.end(); ++jt) {
				std::vector<AnimFrame>& animFrames = (*jt->AnimFrames);
				for (std::vector<AnimFrame>::iterator kt = animFrames.begin(); kt != animFrames.end(); ++kt)
					SpriteTexturesSortedBySize.push_back(&(*kt));
			}
		}
	}
	std::sort(SpriteTexturesSortedBySize.begin(), SpriteTexturesSortedBySize.end(), AnimFrame::SortBySize);

	unsigned int hardwareMaximumTextureSize = sf::Texture::getMaximumSize(); //todo move somewhere else?
	for (std::vector<AnimFrame*>::iterator it = SpriteTexturesSortedBySize.begin(); it != SpriteTexturesSortedBySize.end(); ++it) {
		if (!(*it)->SmallerThan(hardwareMaximumTextureSize))
			continue;

		for (unsigned int textureID = 0; ; ++textureID) {
			while (SpriteTextures.size() <= textureID) { //SpriteTextures and SpriteTrees should always be the same size
				sf::Texture* newTexture = new sf::Texture();
				newTexture->create(hardwareMaximumTextureSize, hardwareMaximumTextureSize);
				SpriteTextures.push_back(newTexture);

				SpriteTreeNode* newNode = new SpriteTreeNode;
				newNode->rectangle = SpriteCoordinateRectangle(0, 0, hardwareMaximumTextureSize, hardwareMaximumTextureSize);
				SpriteTrees.push_back(newNode);
			}

			if (SpriteTrees[textureID] == nullptr)
				continue;

			SpriteCoordinateRectangle* textureCoordinates = SpriteTrees[textureID]->placeSprite((*it)->Width, (*it)->Height);
			if (textureCoordinates != nullptr) {
				(*it)->AssignTextureDetails(textureID, textureCoordinates);
				break;
			}
		}
	}

	for (unsigned int textureID = 0; textureID < SpriteTrees.size(); ++textureID) {
		delete SpriteTrees[textureID]; //deletes its branches recursively
		SpriteTrees[textureID] = nullptr;
	}
	
	//SpriteTextures[0]->copyToImage().saveToFile("C:\\Games\\Jazz2\\SpriteTexture.png");

	return true;
}

bool AnimFile::ReadStream(std::ifstream& file) {
	if (FileSize <= SpecificFileTypeHeaderSize())
		ShowErrorMessage(L"Invalid header (too small)");
	else if (!ReadSpecificFileHeader(file))
		;//method should include its own debug messages
	else {
		file.read((char*)SetAddresses.data(), sizeof(unsigned int) * SetAddresses.size());
		for (std::set<int>::const_iterator it = AnimSetIDs.begin(); it != AnimSetIDs.end(); ++it) {
			//OutputDebugStringF(L"%d", *it);
			if (AnimationSets.size() < size_t(1 + *it))
				AnimationSets.resize(1 + *it);
			file.seekg(SetAddresses[*it], std::ios_base::beg);
			AnimationSets[*it] = new AnimSet(file);
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

void AnimFrame::AssignTextureDetails(unsigned int t, const SpriteCoordinateRectangle* const textureCoordinates) {
	AssignTextureCoordinates(textureCoordinates);

	(Texture = SpriteTextures[Quad.TextureID = t])->update(
		(sf::Uint8*)Image.data(),
		textureCoordinates->width,
		textureCoordinates->height,
		textureCoordinates->left,
		textureCoordinates->top
	);
	Image.resize(0); Image.shrink_to_fit();
}


SpriteCoordinateRectangle * SpriteTreeNode::placeSprite(const unsigned int width, const unsigned int height) {
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
	memcpy((char*)this, frameInfoData, offsetof(AnimFrame, ImageAddress) + sizeof(ImageAddress));
	frameInfoData += offsetof(AnimFrame, ImageAddress) + sizeof(ImageAddress) * 2; //skip MaskAddress
	ImageAddress += int(imageData);

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
void AnimFrame::Draw(Layer& layer, int x, int y, bool flipX, bool flipY) const
{
	quad repositionedQuad(Quad);
	if (flipX)
		repositionedQuad.flipHorizontally();
	if (flipY)
		repositionedQuad.flipVertically();
	repositionedQuad.positionPositionAt(x + (!flipX ? HotspotX : -(Width+HotspotX)), y + (!flipY ? HotspotY : -(Height + HotspotY)));
	layer.DrawQuad(repositionedQuad, Texture);
}
void AnimFrame::DrawRectangle(Layer& layer, int x, int y, int width, int height, sf::Uint8 color)
{
	quad rectangleQuad(static_cast<float>(width), static_cast<float>(height));
	const sf::Vector2f texCoords(static_cast<float>(color), static_cast<float>(pallineXPOSTOINDEX));
	rectangleQuad.positionPositionAt(x, y);
	for (int i = 0; i < 4; ++i)
		rectangleQuad.vertices[i].texCoords = texCoords;
	layer.DrawQuad(rectangleQuad, PaletteTexture);
}
void AnimFrame::DrawPixel(Layer& layer, int x, int y, sf::Uint8 color)
{
	return DrawRectangle(layer, x, y, 1, 1, color);
}
Animation::Animation(const sf::Uint8*& animInfoData, const sf::Uint8*& frameInfoData, const sf::Uint8* const imageData)
{
	memcpy((char*)this, animInfoData, sizeof(Animation));
	animInfoData += sizeof(Animation);

	AnimFrames = std::move(std::unique_ptr<std::vector<AnimFrame>>(new std::vector<AnimFrame>));
	for (int frameID = 0; frameID < FrameCount; ++frameID)
		AnimFrames->push_back(AnimFrame(frameInfoData, imageData));
}
AnimSet::AnimSet(std::ifstream& file)
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
	//const sf::Uint8* sampleData = uncompressedData[3].data(); //todo
	for (int animationID = 0; animationID < AnimationCount; ++animationID)
		Animations.push_back(Animation(animInfoData, frameInfoData, imageData));
}

