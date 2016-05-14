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
	ObjectList* ObjectInitializationList;
	KeyStates Keys;
public:

	Lattice();
	int StartGame(int argc, char *argv[]);
	void SetDefaultAnimList(PreloadedAnimationsList&);
	void SetObjectActivityFunction(ObjectActivityFunction&);
	void SetObjectList(ObjectList*);

	void ProcessInput();
	void Update();
	void Render(double leftoverTimeElapsed);
};