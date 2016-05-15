#pragma once
#include "BunnyObject.h"

class Bunny : public BunnyObject {
	bool KeyUp, KeyRight, KeyDown, KeyLeft, KeyFire, KeySelect, KeyRun, KeyJump;

	void ProcessInput(GameState&);
	void Behave(GameState&) override;
	//void Draw(Layer*) const override;
public:
	Bunny(ObjectStartPos& objStart);
};
