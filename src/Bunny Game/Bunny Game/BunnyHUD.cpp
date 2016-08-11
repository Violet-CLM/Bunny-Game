#include "BunnyHUD.h"
#include "BunnyObjectList.h"
#include "BunnyVersionDependentStuff.h"
#include "Resources.h"

void DrawPlayerHUD(VertexCollectionQueue& Sprites)
{
	const AnimFrame& HeartFrame = (*AnimationSets[GetVersionSpecificAnimationID(AnimSets::Pickups, false)]->Animations[41].AnimFrames)[0];
	for (int i = 0; i < 5; ++i)
		HeartFrame.Draw(Sprites, SpriteMode::Paletted, WINDOW_WIDTH_PIXELS - 12 - i*16, 8);
}
