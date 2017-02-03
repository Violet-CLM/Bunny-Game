#include "BunnyHUD.h"
#include "BunnyObjectList.h"
#include "Bunny.h"
#include "BunnyVersionDependentStuff.h"
#include "BunnyWeapons.h"
#include "Resources.h"
#include "CharStrings.h"
#include "Game.h"

void DrawPlayerHUD(VertexCollectionQueue& Sprites, unsigned int GameTicks) {
	const struct { //using resolution width >= 400 values
		const std::vector<AnimFrame>& largerFont = *AnimationSets[GetVersionSpecificAnimationID(AnimSets::Font)]->Animations[2].AnimFrames;
		const std::vector<AnimFrame>& smallerFont = *AnimationSets[GetVersionSpecificAnimationID(AnimSets::Font)]->Animations[0].AnimFrames;
		unsigned int lineHeightShift = 1;
		unsigned int lineHeight = 10;
	} hm; //recreating this struct makes it easier to paste in JJ2+ code
	const std::vector<AnimFrame>& smallFont = *AnimationSets[GetVersionSpecificAnimationID(AnimSets::Font)]->Animations[1].AnimFrames;
	char buffer[16];

	const auto& play = Players[0]; //no splitscreen support at present
	//for (const auto& play : Players) if (play.Object != nullptr) {
		const auto& playerObject = *play.Object;

		{ //score
			/*
			int scoreDiff = play->score - play->lastScoreDisplay;
			if (scoreDiff > 50) {
				play->lastScoreDisplay += 50;
			} else if (scoreDiff > 10) {
				play->lastScoreDisplay += 10;
			} else if (scoreDiff > 2) {
				play->lastScoreDisplay += 2;
			} else {
				play->lastScoreDisplay = play->score;
			}
			*/
			sprintf_s(buffer, "%07d", playerObject.PlayerProperties.Score);
			WriteText(Sprites, 4, 12, buffer, hm.smallerFont);
		}

		{ //health
			const AnimFrame& HeartFrame = AnimFrame::Get(GetVersionSpecificAnimationID(AnimSets::Pickups), 41, 0);
			for (int i = playerObject.Health - 1; i >= 0; --i)
				Sprites.AppendSprite(SpriteMode::Paletted, WINDOW_WIDTH_PIXELS - 12 - i*16, 8, HeartFrame);
		}
		
		{ //lives
			Sprites.AppendSprite(SpriteMode::Paletted, 0, WINDOW_HEIGHT_PIXELS, AnimFrame::GetLimited(GetVersionSpecificAnimationID(AnimSets::Faces), 3 + playerObject.PlayerProperties.CharacterIndex, GameTicks / 6));
			sprintf_s(buffer, "x%u", playerObject.PlayerProperties.Lives);
			WriteText(Sprites, 32, WINDOW_HEIGHT_PIXELS - hm.lineHeight - 4, buffer, hm.smallerFont);
		}

		{ //ammo
			const int ammo_xPos = WINDOW_WIDTH_PIXELS - (40 << hm.lineHeightShift);
			const int ammo_yPos = WINDOW_HEIGHT_PIXELS - hm.lineHeight - 4;
			const int ammoID = playerObject.fireType;
			int animID;
			if (ammoID != Weapon::Blaster) {
				animID = AmmoIconAnimIDs[ammoID] - playerObject.PlayerProperties.Powerups[ammoID]; //this is correct for everything other than TNT, which... shouldn't be powered up
			} else switch (playerObject.PlayerProperties.CharacterIndex) {
				case char2SPAZ:
					animID = 19; //wrong, todo
					break;
				//case char2LORI:
//					break;
				default: //jazz etc.
					animID = 18; //wrong, todo
					break;
			}
			Sprites.AppendSprite(SpriteMode::Paletted, ammo_xPos - 8, ammo_yPos, AnimFrame::GetLimited(AnimSets::Ammo, animID, GameTicks >> 2));
		
			if (ammoID == 0) {
				sprintf_s(buffer, "x^");
			} else {
				int ammo = playerObject.PlayerProperties.Ammo[ammoID];
				if (
					ammoID == Weapon::Toaster
#ifdef BETAPEPPER
					|| AmmoID == Weapon::Gun8
#endif
				) ammo >>= 5;
					sprintf_s(buffer, "x%u", ammo);
			}
			WriteText(Sprites, ammo_xPos, ammo_yPos, buffer, hm.smallerFont);
		}

#ifdef SHOW_FPS
		{ //fps
			const int info_xPos = WINDOW_WIDTH_PIXELS / 2;
			int info_yPos = WINDOW_HEIGHT_PIXELS - 30;
			sprintf_s(buffer, "fps %3u", std::min(FPS_MAX, FPS));
			WriteText(Sprites, info_xPos, info_yPos, buffer, smallFont);
		}
#endif
	//}
}
