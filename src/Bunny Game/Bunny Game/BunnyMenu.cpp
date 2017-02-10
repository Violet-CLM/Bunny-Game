#include "BunnyMenu.h"
#include "BunnyMisc.h"
#include "BunnyShaders.h"
#include "BunnyObjectList.h"
#include "BunnyVersionDependentStuff.h"
#include "CharStrings.h"
#include "PostProcessing.h"

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
BunnyMenu::BunnyMenu() {
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
	}
}

void BunnyMenu::Update(const KeyStates& keys) { //todo obviously
	LightingSprites.Clear();
	Sprites.Clear();

	WriteText(Sprites, 236, 210, "Bunny Game", *Fonts[1]);
	WriteText(Sprites, 214, 270, "Press Enter to begin", *Fonts[0]);
	DrawLightToLightBuffer(LightType::Ring, 150, 68, sf::Vector2f(WINDOW_WIDTH_PIXELS / 2, WINDOW_HEIGHT_PIXELS / 2));

	if (keys.KeyPressed(sf::Keyboard::Return)) {
		ReplaceWithNewStage(Level::LoadLevel(std::wstring(L"Diam3")));
	}
}

void BunnyMenu::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	DrawMenuBG(target);
	target.draw(Sprites, states);
}
