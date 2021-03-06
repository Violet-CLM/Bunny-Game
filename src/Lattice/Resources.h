#pragma once

#include <set>
#include <unordered_map>
#include <memory>
#include "SFML/Audio.hpp"
#include "Files.h"
#include "Tile.h"
#include "Constants.h"
#include "Layer.h"

class SpriteManager;
class AnimSet;
typedef sf::Rect<unsigned int> SpriteCoordinateRectangle;

class AnimFile : public JazzFile {
	const static sf::Uint32 ProperHeaderSignature = 0x00BEBA00;
private:
	struct AnimFileHeader {
		//private:
		char Magic[FILE_HEADER_MAGICLENGTH];// "ALIB"
		sf::Uint32 Signature;
		sf::Uint32 HeaderSize;
		FileVersion Version;
		FileVersion Version2;
		sf::Uint32 FileSize;
		sf::Uint32 CRC32;

		bool Matches();
	public:
		sf::Uint32 SetCount;

		bool Read(std::ifstream&);
	};
	static_assert(sizeof(AnimFileHeader) == 28, "AnimFileHeader incorrect size!");

	unsigned int SpecificFileTypeHeaderSize() override;
	bool ReadSpecificFileHeader(std::ifstream&) override;
	bool ReadStream(std::ifstream&) override;
	AnimFile(std::wstring&, const PreloadedAnimationsList&, std::vector<AnimSet>&);

	const PreloadedAnimationsList& AnimSetIDs;
	std::vector<AnimSet>& AnimationSets;
	std::vector<unsigned int> SetAddresses;
public:

	static bool ReadAnims(std::wstring& Filepath, const PreloadedAnimationsList&, SpriteManager&);
};

class VertexCollectionQueue;
class AnimFrame {
friend class VertexCollectionQueue;

public:
	sf::Uint16 Width;
	sf::Uint16 Height;
	sf::Int16 ColdspotX;    // Relative to hotspot
	sf::Int16 ColdspotY;    // Relative to hotspot
	sf::Int16 HotspotX;
	sf::Int16 HotspotY;
private:
	sf::Int16 GunspotX;     // Relative to hotspot
	sf::Int16 GunspotY;     // Relative to hotspot
	union {
		const sf::Uint8* ImageAddress;  // Address in Data3 where image starts
		sf::Texture* Texture;
	};
	std::vector<sf::Uint32> Image;
	unsigned int Area; //for use in filling the sprite texture/s efficiently, a simple product of the frame's width/height
	quad Quad;

	void AssignTextureCoordinates(const SpriteCoordinateRectangle* const textureCoordinates);
public:
	void AssignTextureDetails(unsigned int t, const SpriteCoordinateRectangle* const textureCoordinates, std::vector<std::unique_ptr<sf::Texture>>&, bool saveImagesInCPU);
	sf::Uint32* CreateImage(unsigned int w, unsigned int h);

	AnimFrame(){}
	AnimFrame(const sf::Uint8*&, const sf::Uint8* const);
	static bool SortBySize(const AnimFrame* a, const AnimFrame* b) { return a->Area > b->Area; }
	bool SmallerThan(unsigned int) const;

	void MovePositionToGunSpotX(float&, bool) const;
	void MovePositionToGunSpotY(float&, bool) const;

	std::vector<sf::Uint8> Image8Bit;
};
class Animation {
private:
	sf::Uint16 FrameCount;
	sf::Uint16 FPS;
	const static size_t NumberOfBytesToReadFromFile = 8;
public:
	std::unique_ptr<std::vector<AnimFrame>> AnimFrames;

	Animation(const sf::Uint8*&, const sf::Uint8*&, const sf::Uint8* const);
};

class AnimSet {
private:
	char AnimationCount;   // Number of animations in set
	char SampleCount;      // Number of sound samples in set
	void LoadSample(const sf::Uint8*&);
public:
	std::vector<Animation> Animations;
	std::vector<sf::SoundBuffer> Samples;
	sf::Sound& StartSound(unsigned int) const;
	sf::Sound& StartSound(unsigned int, sf::Vector2f, unsigned int = 0, unsigned int = 0) const;
	void LoadFromFile(std::ifstream&);
};

class SpriteManager {
friend class Stage;
friend class AnimFile;
friend class GameObject;
	struct SpriteTreeNode {
		SpriteTreeNode* firstChild;
		union {
			SpriteTreeNode* secondChild;
			bool used;
		};
		SpriteCoordinateRectangle rectangle;

		SpriteTreeNode() : firstChild(nullptr), secondChild(nullptr) {}
		~SpriteTreeNode() {
			if (firstChild != nullptr) {
				delete firstChild;
				delete secondChild;
			}
		}

		SpriteCoordinateRectangle* placeSprite(const unsigned int width, const unsigned int height);
	};
	std::vector<SpriteTreeNode*> SpriteTrees;
	std::vector<std::unique_ptr<sf::Texture>> SpriteTextures;
	std::vector<AnimFrame*> SpriteTexturesSortedBySize;
	std::vector<AnimSet> AnimationSets;

public:
	void AddFrame(AnimFrame&);
	void CreateAndAssignTextures(bool);
	void CreateAndAssignTextureForSingleFrame(AnimFrame&);
	void Clear();

	AnimFrame& GetFrame(int, int, int) const;
	AnimFrame& GetFrameLimited(int, int, int) const; //frameID is moduloed to fit
	
	~SpriteManager();
};

//AnimFrame SpritePropertyList[MAXFRAMES];
//std::unordered_map<unsigned int, AnimFrame> LightingProperties;