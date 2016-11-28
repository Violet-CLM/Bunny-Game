#include "BunnyHUD.h"
#include "BunnyObjectList.h"
#include "Bunny.h"
#include "BunnyVersionDependentStuff.h"
#include "BunnyWeapons.h"
#include "Resources.h"

void DrawPlayerHUD(VertexCollectionQueue& Sprites, unsigned int GameTicks)
{
	const AnimFrame& HeartFrame = AnimFrame::Get(GetVersionSpecificAnimationID(AnimSets::Pickups), 41, 0);
	const auto& play = Players[0]; //no splitscreen support at present
	//for (const auto& play : Players) if (play.Object != nullptr) {
		//health
		const auto& playerObject = *play.Object;
		for (int i = playerObject.Health - 1; i >= 0; --i)
			Sprites.AppendSprite(SpriteMode::Paletted, WINDOW_WIDTH_PIXELS - 12 - i*16, 8, HeartFrame);
		
		//lives
		Sprites.AppendSprite(SpriteMode::Paletted, 0, WINDOW_HEIGHT_PIXELS, AnimFrame::GetLimited(GetVersionSpecificAnimationID(AnimSets::Faces), 3 + playerObject.PlayerProperties.CharacterIndex, GameTicks / 6));

		//ammo
		
		const int ammo_xPos = WINDOW_WIDTH_PIXELS;// - (40 << hm.lineHeightShift);
		const int ammo_yPos = WINDOW_HEIGHT_PIXELS;// - hm.lineHeight - 4;
		const int ammoID = playerObject.fireType;
		int animID;
		if (ammoID != Weapon::Blaster) {
			animID = AmmoIconAnimIDs[ammoID] - playerObject.PlayerProperties.Powerups[ammoID]; //this is correct for everything other than TNT, which... shouldn't be powered up
		} else {
			animID = 18; //wrong, todo
		}
		Sprites.AppendSprite(SpriteMode::Paletted, ammo_xPos, ammo_yPos - 8, AnimFrame::GetLimited(AnimSets::Ammo, animID, GameTicks >> 2));
	//}
}
