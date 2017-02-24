#pragma once
#include "Data.h"
#include "Level.h"
#include "Lattice.h"
#include "CharStrings.h"
#include "BunnyShaders.h"

class MenuString {
private:
	const TtextAppearance& appearance;
	int xPos, yPos, animSpeed;
	std::string Text;
	bool large;
public:
	MenuString(int x, int y, const char* t, const TtextAppearance& a, bool l = true) : xPos(x), yPos(y), Text(t), appearance(a), large(l) {}
	MenuString(int x, int y, const std::string& t, const TtextAppearance& a, bool l = true) : xPos(x), yPos(y), Text(t), appearance(a), large(l) {}
	void Draw(const WriteCharacter& WriteCharFunc, const std::vector<AnimFrame>** Fonts, unsigned int GameTicks) const;

};
typedef std::vector<MenuString> MenuStrings;

class MenuScreen {
protected:
	int SelectedItem = 0, ItemCount;
	//void MoveSelectionUpOrDown();
	//static const TtextAppearance& GetAnimatedness(bool);
	//const TtextAppearance& GetAnimatedness(int) const;
	MenuScreen(const char* t = nullptr, int ic = 0) : ItemCount(ic), Title(t) {}
public:
	const char* Title;
	virtual void Draw(MenuStrings&) const = 0;
	virtual MenuScreen* Behave(const KeyStates&) = 0;
};

namespace BunnyMenuTextureIDs { enum {
	Image128, Image32, Image16, LASTSAVEDASINDICES = Image16, Palette, LAST
};}
class BunnyMenu : public Stage {
	VertexCollectionQueue Sprites, ShadowSprites;
	SpriteMode ShadowMode;
	WriteCharacter writeCharFunc;
	const std::vector<AnimFrame>* Fonts[2];
	const AnimFrame* Logo;
	unsigned int GameTicks = 0;

	std::unique_ptr<MenuScreen> Screen;

	std::array<DataTarget, BunnyMenuTextureIDs::LAST> DataTargets;
	std::array<sf::Texture, BunnyMenuTextureIDs::LAST> Textures;
	
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
public:
	void Update(const KeyStates&) override;
	BunnyMenu();
};

class DummyMenu : public MenuScreen {
	void Draw(MenuStrings&) const override;
	MenuScreen* Behave(const KeyStates&) override;
public:
	DummyMenu();
};
