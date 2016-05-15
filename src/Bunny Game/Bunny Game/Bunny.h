#pragma once
#include "BunnyObject.h"

class Bunny : public BunnyObject {
	bool KeyUp, KeyRight, KeyDown, KeyLeft, KeyFire, KeySelect, KeyRun, KeyJump;


	void GetInput(const KeyStates&);		//461C20
	void ProcessInput();					//435AC0
	void DoLandscapeCollision();			//437D30
	void DoZoneDetection();
	void ProcessAction();					//4348E0
	void AdjustViewpoint(GameState&) const;	//43E560

	void Behave(GameState&) override;
	//void Draw(Layer*) const override;
public:
	Bunny(ObjectStartPos& objStart);
};
