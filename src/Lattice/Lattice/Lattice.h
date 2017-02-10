#pragma once
#include "Constants.h"
#include "Level.h"

#define FPS_MAX 60

class KeyStates {
friend class Lattice;
	bool Previous[sf::Keyboard::KeyCount];
	bool Current[sf::Keyboard::KeyCount];

	void Update() {
		memcpy(Previous, Current, sizeof(Current));
	}
	void SetKey(sf::Keyboard::Key key, bool setTo) {
		if (key <= sf::Keyboard::Unknown) return;
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
	sf::RenderWindow* Window;
	sf::RenderTexture VideoBuffer;
	Level* CurrentLevel; //todo
	KeyStates Keys;

	static unsigned int RenderFrame;

	void LoadLevel(std::wstring&);
	bool ProcessInput();
	void Update();
	void Render(double);
public:
#ifdef SHOW_FPS
	static int FPS;
#endif

	static const ObjectList* ObjectInitializationList;
	static unsigned int GetFramesElapsed();
	Lattice(sf::RenderWindow&, std::wstring&, const ObjectList&);
};
