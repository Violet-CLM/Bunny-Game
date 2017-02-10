#include "BunnyEffects.h"
#include "BunnyObjectList.h"
#include "BunnyMisc.h"

void Explosion::Behave(GameState& gameState)
{
	if ((++counter & 3) == 0 && ++FrameID >= GetFrameCount())
		Delete();
}

Explosion& BunnyObject::AddExplosion(int setID, int animID, bool makeChild)
{
	Explosion& newExplosion = static_cast<Explosion&>(AddObject(EventIDs::EXPLOSION, PositionX, PositionY, makeChild));
	newExplosion.Set = &HostLevel.GetAnimSet(setID);
	newExplosion.AnimID = animID;
	return newExplosion;
}

void FlickerLight::Draw(Layer *) const {
	DrawObjectToLightBuffer(*this);
	//todo sound
}
