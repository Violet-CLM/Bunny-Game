#pragma once
#include "Data.h"
#include "Level.h"
#include "Lattice.h"

class BunnyMenu : public Stage {
	SpriteManager spriteManager;
	DataTarget Image128, Image32, Image16, Palette;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
	void Update(const KeyStates&) override;
	BunnyMenu();
};
