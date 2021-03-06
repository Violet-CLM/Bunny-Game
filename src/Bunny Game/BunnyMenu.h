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
	bool large, dark;
public:
	MenuString(int x, int y, const char* t, const TtextAppearance& a, bool l = true, bool d = false) : xPos(x), yPos(y), Text(t), appearance(a), large(l), dark(d) {}
	MenuString(int x, int y, const std::string& t, const TtextAppearance& a, bool l = true, bool d = false) : xPos(x), yPos(y), Text(t), appearance(a), large(l), dark(d) {}
	void Draw(const WriteCharacter* WriteCharFuncs, const std::vector<AnimFrame>** Fonts, unsigned int GameTicks) const;

};
typedef std::vector<MenuString> MenuStrings;

DerivedSpriteMode(DarkSprite) {
public:
	DarkSprite(sf::Uint8 p) : SpriteModeDerivationHelper(Shaders[BunnyShaders::Brightness], p) {}
	void Apply(sf::RenderStates&) const override;
};

class MenuScreen {
protected:
	int SelectedItem = 0, ItemCount;

	int GetVerticalInput(const KeyStates&) const;
	int GetHorizontalInput(const KeyStates&) const;
	bool GetAdvance(const KeyStates&) const;
	bool GetRetreat(const KeyStates&) const;
	void MoveSelectionUpOrDown(const KeyStates&);

	static const TtextAppearance& GetAnimatedness(bool);
	const TtextAppearance& GetAnimatedness(int) const;

	MenuScreen(const char* t = nullptr, int ic = 0) : ItemCount(ic), Title(t) {}
public:
	static std::wstring levelFilename;
	const char* Title;
	virtual void Draw(MenuStrings&) const = 0;
	virtual MenuScreen* Behave(const KeyStates&) = 0;
};

namespace BunnyMenuTextureIDs { enum {
	Image128, Image32, Image16, LASTSAVEDASINDICES = Image16, Palette, LAST
};}
class BunnyMenu : public Stage {
	VertexCollectionQueue Sprites, ShadowSprites;
	SpriteMode ShadowMode; DarkSprite DarkCharacterMode;
	WriteCharacter writeCharFunc[2];
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

class RootMenu : public MenuScreen {
	void Draw(MenuStrings&) const override;
	MenuScreen* Behave(const KeyStates&) override;
public:
	RootMenu(int startItem = 0);
};
