#pragma once
#include "Constants.h"
#include "Level.h"

#define FPS_MAX 60
extern int FPS;

struct KeyStates {
	bool Previous[sf::Keyboard::KeyCount];
	bool Current[sf::Keyboard::KeyCount];

	void Update() {
		memcpy(Previous, Current, sizeof(Current));
	}
	void SetKey(sf::Keyboard::Key key, bool setTo) {
		Current[key] = setTo;
		Previous[key] = !setTo;
	}
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
#ifdef SHOW_FPS
	int FPS;
#endif
	sf::RenderWindow* Window;
	Level* CurrentLevel; //todo
	KeyStates Keys;

	void LoadLevel(std::wstring&);
	bool ProcessInput();
	void Update();
	void Render(double);
public:
	static const ObjectList* ObjectInitializationList;
	Lattice(sf::RenderWindow&, std::wstring&, const ObjectList&);
};
