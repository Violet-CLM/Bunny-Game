#include "BunnyHUD.h"
#include "BunnyObjectList.h"
#include "BunnyVersionDependentStuff.h"
#include "Resources.h"

void DrawPlayerHUD(VertexCollectionQueue& Sprites)
{
	const AnimFrame& HeartFrame = AnimFrame::Get(GetVersionSpecificAnimationID(AnimSets::Pickups), 41, 0);
	for (int i = 0; i < 5; ++i)
		Sprites.DrawSprite(SpriteMode::Paletted, WINDOW_WIDTH_PIXELS - 12 - i*16, 8, HeartFrame);
}
