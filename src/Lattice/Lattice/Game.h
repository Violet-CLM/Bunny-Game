#pragma once
#include "Level.h"

class KeyStates {
	friend class Lattice;
private:
	bool Previous[sf::Keyboard::KeyCount];
	bool Current[sf::Keyboard::KeyCount];

	void Update() {
		memcpy(Previous, Current, sizeof(Current));
	}
	void SetKey(sf::Keyboard::Key key, bool setTo) {
		Current[key] = setTo;
		Previous[key] = !setTo;
	}
public:
	bool Key(sf::Keyboard::Key key) const {
		return Current[key];
	}
	bool KeyPressed(sf::Keyboard::Key key) const {
		return Current[key] && !Previous[key];
	}
	bool KeyReleased(sf::Keyboard::Key key) const {
		return !Current[key] && Previous[key];
	}
};

class Lattice {
private:
	sf::RenderWindow* Window;

	void LoadLevel(std::wstring&, PreloadedAnimationsList&, ObjectList&);
	void StartGame(sf::RenderWindow& window, std::wstring&);

	Level* Level;
	PreloadedAnimationsList DefaultAnimList;
	ObjectActivityFunction* ActivateObjects;
	ObjectCollisionTestFunction* CollideObjects;
	PaletteTableSetupFunction* SetupPaletteTables;
	ObjectList* ObjectInitializationList;
	KeyStates Keys;

	unsigned int PaletteLineCount;
public:

	Lattice();
	int StartGame(int argc, char *argv[]);
	void DefineShaders(std::vector<std::string>&, PaletteTableSetupFunction*, unsigned int);
	void SetDefaultAnimList(PreloadedAnimationsList&);
	void SetObjectActivityFunction(ObjectActivityFunction&);
	void SetObjectCollisionTestFunction(ObjectCollisionTestFunction&);
	void SetObjectList(ObjectList*);

	void ProcessInput();
	void Update();
	void Render(double leftoverTimeElapsed);
};