#include "BunnyEffects.h"
#include "BunnyObjectList.h"
#include "BunnyMisc.h"

void Explosion::Behave(GameState& gameState)
{
	if ((++counter & 3) == 0 && ++FrameID >= GetFrameCount())
		Delete();
}

Explosion& Explosion::AddExplosion(GameObject& parent, int setID, int animID, bool makeChild)
{
	Explosion& newExplosion = static_cast<Explosion&>(parent.AddObject(EventIDs::EXPLOSION, parent.PositionX, parent.PositionY, makeChild));
	newExplosion.Set = AnimationSets[setID];
	newExplosion.AnimID = animID;
	return newExplosion;
}
