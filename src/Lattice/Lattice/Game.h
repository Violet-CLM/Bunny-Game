#pragma once
#include "Level.h"

class Lattice {
private:
	sf::RenderWindow* Window;

	void LoadLevel(std::wstring&, PreloadedAnimationsList&, ObjectList&);
	void StartGame(sf::RenderWindow& window, std::wstring&);
public:

	Lattice();
	int StartGame(int argc, char *argv[]);
	void SetDefaultAnimList(PreloadedAnimationsList&);
	void SetObjectActivityFunction(ObjectActivityFunction&);
	void SetObjectList(ObjectList*);

	Level* Level;
	PreloadedAnimationsList DefaultAnimList;
	ObjectActivityFunction* ActivateObjects;
	ObjectList* ObjectInitializationList;

	void ProcessInput();
	void Update();
	void Render(double leftoverTimeElapsed);
};