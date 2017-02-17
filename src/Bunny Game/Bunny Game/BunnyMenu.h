#pragma once
#include "Data.h"
#include "Level.h"
#include "Lattice.h"
#include "CharStrings.h"

namespace BunnyMenuTextureIDs { enum {
	Image128, Image32, Image16, LASTSAVEDASINDICES = Image16, Palette, LAST
};}
class BunnyMenu : public Stage {
	VertexCollectionQueue Sprites;
	WriteCharacter writeCharFunc = GetWriteCharacterFunction(Sprites);
	const std::vector<AnimFrame>* Fonts[2];
	unsigned int GameTicks = 0;

	std::array<DataTarget, BunnyMenuTextureIDs::LAST> DataTargets;
	std::array<sf::Texture, BunnyMenuTextureIDs::LAST> Textures;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
	void Update(const KeyStates&) override;
	BunnyMenu();
};
