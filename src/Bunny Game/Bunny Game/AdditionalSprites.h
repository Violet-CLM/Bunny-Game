#pragma once
#include "Resources.h"

namespace AdditionalSprites { enum {
	MenuBottomGradient,
	LAST
};}
extern std::vector<AnimFrame> AdditionalAnimFrames;
extern SpriteManager EffectSprites;

extern VertexCollectionQueue MenuBottomGradientVertices; //drawn unchanging to every frame to any menu screen, so it can reasonably be generated in advance

void InitCreateAdditionalSprites();
