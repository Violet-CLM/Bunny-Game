#include "AdditionalSprites.h"

std::vector<AnimFrame> AdditionalAnimFrames(AdditionalSprites::LAST);
SpriteManager EffectSprites;
VertexCollectionQueue MenuBottomGradientVertices;

void InitCreateAdditionalSprites() {
	const unsigned int MenuBottomGradientHeight = 20;

	sf::Uint32* image;
	AnimFrame* frame;

	frame = &AdditionalAnimFrames[AdditionalSprites::MenuBottomGradient];
	frame->HotspotX = frame->HotspotY = 0;
	image = frame->CreateImage(1, MenuBottomGradientHeight);
	for (unsigned int i = MenuBottomGradientHeight; i > 0; --i)
		*image++ = (i << 3) | (i << 11) | (i << 19) | (0x80 << 24); //half-opaque, with whiteness ranging from 8 to 160


	for (auto& it : AdditionalAnimFrames)
		EffectSprites.AddFrame(it);
	EffectSprites.CreateAndAssignTextures();
	
	MenuBottomGradientVertices.AppendResizedSprite(SpriteMode::Normal, 0, WINDOW_HEIGHT_PIXELS - MenuBottomGradientHeight, AdditionalAnimFrames[AdditionalSprites::MenuBottomGradient], WINDOW_WIDTH_PIXELS, 1);
}
