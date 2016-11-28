#include "BunnyHUD.h"
#include "BunnyObjectList.h"
#include "Bunny.h"
#include "BunnyVersionDependentStuff.h"
#include "Resources.h"

void DrawPlayerHUD(VertexCollectionQueue& Sprites)
{
	const AnimFrame& HeartFrame = AnimFrame::Get(GetVersionSpecificAnimationID(AnimSets::Pickups), 41, 0);
	for (const auto& play : Players) if (play.Object != nullptr) {
		const auto& playerObject = *play.Object;
		for (int i = playerObject.Health - 1; i >= 0; --i)
			Sprites.AppendSprite(SpriteMode::Paletted, WINDOW_WIDTH_PIXELS - 12 - i*16, 8, HeartFrame);
	}
}
