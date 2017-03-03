#include "BunnyMenu.h"
#include "BunnyMisc.h"
#include "BunnyObjectList.h"
#include "BunnyVersionDependentStuff.h"
#include "PostProcessing.h"
#include "J2S.h"

std::wstring MenuScreen::levelFilename = L"";

void AssignImagesToMenuShader(const std::array<sf::Texture, BunnyMenuTextureIDs::LAST>& Textures) {
	Shaders[BunnyShaders::MenuBG]->setUniform("texture128", Textures[BunnyMenuTextureIDs::Image128]);
	Shaders[BunnyShaders::MenuBG]->setUniform("texture32", Textures[BunnyMenuTextureIDs::Image32]);
	Shaders[BunnyShaders::MenuBG]->setUniform("texture16", Textures[BunnyMenuTextureIDs::Image16]);
	//Shaders[BunnyShaders::MenuBG]->setUniform("palette", Textures[BunnyMenuTextureIDs::Palette]);
}
void DrawMenuBG(sf::RenderTarget& target) {
	auto time = Lattice::GetFramesElapsed() << 4;
	{
		const float size = ((1 << 30) / ((cosTable(time >> 4) * 2048.f) + 0x3000)) / 65536.f;
		sf::Transform transform;
		transform
			.rotate((time >> 3) * 360.f / 1024.f + 90.f)
			.scale(size, size);
		Shaders[BunnyShaders::MenuBG]->setUniform("transform16", sf::Glsl::Mat4(transform));
	} {
		const float size = ((1 << 30) / ((sinTable(time >> 4) * 2048.f) + 0x3000)) / 65536.f;
		time += 768 << 5;
		sf::Transform transform;
		transform
			.rotate((time >> 5) * 360.f / 1024.f)
			.scale(size, size);
		Shaders[BunnyShaders::MenuBG]->setUniform("transform128", sf::Glsl::Mat4(transform));
	} {
		const float v0 = (time >> 3) * 3.14159265359f / 512.f;
		Shaders[BunnyShaders::MenuBG]->setUniform("transform32", sf::Glsl::Vec4(sin(v0) * 64.f, cos(v0 / 2.f) * -64.f, 0, 0));
	}
	target.draw(FullScreenQuad.vertices, 4, sf::Quads, MenuBGRenderStates);
}

#include "Windows.h"
#include "Misc.h"
BunnyMenu::BunnyMenu() : ShadowMode(Shaders[BunnyShaders::Shadow], 0), DarkCharacterMode(82) {
	LoadDataFromFile(
		L"Data.j2d",
		{"Menu.Texture.128x128", "Menu.Texture.32x32", "Menu.Texture.16x16", "Menu.Palette"},
		{&DataTargets[BunnyMenuTextureIDs::Image128], &DataTargets[BunnyMenuTextureIDs::Image32], &DataTargets[BunnyMenuTextureIDs::Image16], &DataTargets[BunnyMenuTextureIDs::Palette]}
	);

	const static sf::Vector2u ImageDimensions[BunnyMenuTextureIDs::LAST] = {{128, 128}, {32, 32}, {16, 16}, {COLORSPERPALETTE, 1}};
	for (unsigned int imageID = 0; imageID <= BunnyMenuTextureIDs::LASTSAVEDASINDICES; ++imageID) { //most images in a .j2d file will be stored using only one byte per pixel, which makes sense but does need to be converted in order to be used in the GPU
		auto& paletteIndices = DataTargets[imageID];
		std::vector<sf::Color> buffer(paletteIndices.size());
		for (int j = paletteIndices.size() - 1; j >= 0; --j)
			if (buffer[j].r = paletteIndices[j])
				buffer[j].g = 255u; //opaque, but alpha is giving me trouble with this shader for whatever stupid reason
		Textures[imageID].create(ImageDimensions[imageID].x, ImageDimensions[imageID].y);
		Textures[imageID].update((sf::Uint8*)buffer.data());
		Textures[imageID].setRepeated(true);
		paletteIndices.resize(0); paletteIndices.shrink_to_fit(); //no longer needed
	}

	GeneratePaletteTexture(Textures[BunnyMenuTextureIDs::Palette], DataTargets[BunnyMenuTextureIDs::Palette].data());
	DataTargets[BunnyMenuTextureIDs::Palette].resize(0); DataTargets[BunnyMenuTextureIDs::Palette].shrink_to_fit();
	AssignImagesToMenuShader(Textures);

	AnimFile::ReadAnims(std::wstring(L"Anims.j2a"), {
		GetVersionSpecificAnimationID(AnimSets::Menu),
		GetVersionSpecificAnimationID(AnimSets::MenuFont),
		GetVersionSpecificAnimationID(AnimSets::MenuSounds)
	}, spriteManager);

	{
		const AnimSet& fontSet = GetAnimSet(GetVersionSpecificAnimationID(AnimSets::MenuFont));
		for (int i = 0; i < 2; ++i)
			Fonts[i] = fontSet.Animations[i ^ 1].AnimFrames.get();
		Logo = fontSet.Animations[2].AnimFrames.get()->data();
	}

	writeCharFunc[0] = [this](const AnimFrame& frame, sf::Uint8 spriteParam, int x, int y) {
		//todo alter position somehow based on menu transitions
		ShadowSprites.AppendSprite(ShadowMode, x, y+4, frame);
		Sprites.AppendSprite(SpriteMode(Shaders[BunnyShaders::Palshift], spriteParam), x,y, frame); //standard behavior
	};
	writeCharFunc[1] = [this](const AnimFrame& frame, sf::Uint8, int x, int y) { //dark
		//todo alter position somehow based on menu transitions
		ShadowSprites.AppendSprite(ShadowMode, x, y+4, frame);
		Sprites.AppendSprite(DarkCharacterMode, x,y, frame);
	};

	ClearLightingBuffer();
	CurrentStageType = StageType::Menu;

	Screen.reset(new RootMenu());
}

void BunnyMenu::Update(const KeyStates& keys) {
	++GameTicks;

	const auto CurrentMenuScreen = Screen.get();
	if (CurrentMenuScreen != nullptr) {
		const auto NextMenuScreen = CurrentMenuScreen->Behave(keys);
		if (NextMenuScreen == CurrentMenuScreen) { //no change, so it's time to draw this
			LightingSprites.Clear();
			ShadowSprites.Clear();
			Sprites.Clear();
			
			ShadowSprites.AppendSprite(ShadowMode, 7,7, *Logo);
			Sprites.AppendSprite(SpriteMode::Paletted, -1,-1, *Logo);
			const static sf::Vector2f LogoLightPosition(32, 16);

			MenuStrings strings;
			CurrentMenuScreen->Draw(strings);
			if (CurrentMenuScreen->Title != nullptr)
				strings.emplace_back( TtextAppearance::DefaultRightAlign, WINDOW_HEIGHT_PIXELS - 11, CurrentMenuScreen->Title, TtextAppearance::defaultMenuScreenTitle, true);
			for (const auto& it : strings)
				it.Draw(writeCharFunc, Fonts, GameTicks);
		} else if (NextMenuScreen == nullptr) { //quit menu system
			if (!MenuScreen::levelFilename.empty()) {
				ReplaceWithNewStage(Level::LoadLevel(MenuScreen::levelFilename));
				MenuScreen::levelFilename.clear(); //in case the player comes back to the menu system later
			} else
				DeleteCurrentStage(); //end game, pretty much
		} else //change menu screen
			Screen.reset(NextMenuScreen);
	}
}

void BunnyMenu::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	DrawMenuBG(target);
	target.draw(ShadowSprites, states);
	target.draw(Sprites, states);
}



const TtextAppearance& MenuScreen::GetAnimatedness(bool yes) {
	return yes ? TtextAppearance::defaultMenuSpinFast : TtextAppearance::defaultNormal;
}
const TtextAppearance& MenuScreen::GetAnimatedness(int i) const {
	return GetAnimatedness(i == SelectedItem);
}


int MenuScreen::GetVerticalInput(const KeyStates& keys) const {
	if (keys.KeyPressed(sf::Keyboard::Up))
		return -1;
	if (keys.KeyPressed(sf::Keyboard::Down))
		return 1;
	return 0;
}
int MenuScreen::GetHorizontalInput(const KeyStates& keys) const {
	if (keys.KeyPressed(sf::Keyboard::Left))
		return -1;
	if (keys.KeyPressed(sf::Keyboard::Right))
		return 1;
	return 0;
}
bool MenuScreen::GetAdvance(const KeyStates& keys) const {
	return keys.KeyPressed(sf::Keyboard::Return) || keys.KeyPressed(sf::Keyboard::Space);
}
bool MenuScreen::GetRetreat(const KeyStates& keys) const {
	return keys.KeyPressed(sf::Keyboard::Escape);
}
void MenuScreen::MoveSelectionUpOrDown(const KeyStates& keys) {
	if (SelectedItem += GetVerticalInput(keys) < 0)
		SelectedItem = ItemCount - 1;
	else if (SelectedItem >= ItemCount)
		SelectedItem = 0;
}

void MenuString::Draw(const WriteCharacter* WriteCharFuncs, const std::vector<AnimFrame>** Fonts, unsigned int GameTicks) const {
	WriteText(WriteCharFuncs[dark], xPos,yPos, Text.c_str(), *Fonts[large], appearance, GameTicks);
}
void DarkSprite::Apply(sf::RenderStates& states) const {
	if (ParamAsFloat > 0.5f) {
		float brightness = ParamAsFloat * 2;
		Shader->setUniform("param", brightness);
		brightness -= 1;
		Shader->setUniform("brightness", sf::Glsl::Vec4(brightness,brightness,brightness,1.f));
	} else
		Shader->setUniform("param", (0.5f - ParamAsFloat) * 2.0f);

	states.shader = Shader;
	states.blendMode = BlendMode;
}



RootMenu::RootMenu(int startItem) : MenuScreen(GetTranslatedString(StringID::MainMenu), 5) {
	//MenuGlobals->menuLightCount = 2;
	//GeneralGlobals->menuGlowStyle1 = 0;
	//GeneralGlobals->menuGlowStyle2 = 0;
	SelectedItem = startItem;
}
const static bool RootMenuItemsAreAvailable[] = {true, false, false, false, true};
void RootMenu::Draw(MenuStrings& strings) const {
	const int distanceBetweenMenuitems = 22 * WINDOW_HEIGHT_PIXELS / 200;
	int menuYPos = 60 * WINDOW_HEIGHT_PIXELS / 200 - distanceBetweenMenuitems;
	for (int i = 0; i < ItemCount; ++i) {
		strings.emplace_back(TtextAppearance::DefaultCenterAlign, menuYPos += distanceBetweenMenuitems, GetTranslatedString(StringID::StringID(StringID::NewGame + (i != 2 ? i : 5))), GetAnimatedness(i), true, !RootMenuItemsAreAvailable[i]);
		//if (SelectedItem == i)
			//GeneralGlobals->menuGlowYPos = menuYPos << FIXFAC;
	}
	strings.emplace_back(0, WINDOW_HEIGHT_PIXELS - 6, "1.00", TtextAppearance::defaultMenuSpinSlow, false); //todo base this line on some value somewhere

	//GeneralGlobals->menuGlowXPos = *screenWidth << 15;
}
MenuScreen* RootMenu::Behave(const KeyStates& keys) {
	if (GetAdvance(keys)) {
		switch (SelectedItem) {
		case 0:
			levelFilename = L"Diam3"; //todo
			return nullptr;
		default:
			return nullptr; //quit
		}
	} else if (GetVerticalInput(keys))
		SelectedItem ^= 4; //skip over 1,2,3 for now
	else if (GetRetreat(keys))
		SelectedItem = 4;
	return this; //normal return
}
