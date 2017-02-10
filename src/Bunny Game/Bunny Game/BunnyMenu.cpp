#include "BunnyMenu.h"
#include "BunnyObjectList.h"
#include "BunnyVersionDependentStuff.h"

BunnyMenu::BunnyMenu() {
	LoadDataFromFile(
		L"Data.j2d",
		{"Menu.Texture.128x128", "Menu.Texture.32x32", "Menu.Texture.16x16", "Menu.Palette"},
		{&Image128, &Image32, &Image16, &Palette}
	);
	AnimFile::ReadAnims(std::wstring(L"Anims.j2a"), {
		GetVersionSpecificAnimationID(AnimSets::Menu),
		GetVersionSpecificAnimationID(AnimSets::MenuFont),
		GetVersionSpecificAnimationID(AnimSets::MenuSounds)
	}, spriteManager);
}

void BunnyMenu::Update(const KeyStates& keys) { //todo obviously
	if (keys.KeyPressed(sf::Keyboard::Return)) {
		ReplaceWithNewStage(Level::LoadLevel(std::wstring(L"Diam3")));
	}
}

void BunnyMenu::draw(sf::RenderTarget & target,sf::RenderStates states) const { //todo obviously
}
