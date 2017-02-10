#pragma once
#include <stack>
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
friend class Stage;
	static sf::RenderWindow* Window;
	sf::RenderTexture VideoBuffer;
	KeyStates Keys;

	static unsigned int RenderFrame;
	static std::unique_ptr<Stage> StageToDelete;
	static std::stack<std::unique_ptr<Stage>> Stages;

	bool ProcessInput();
	void Update();
	void Render(double);
	bool Loop();
public:
#ifdef SHOW_FPS
	static int FPS;
#endif

	static const ObjectList* ObjectInitializationList;
	static unsigned int GetFramesElapsed();
	static void SetWindowTitle(const sf::String&);
	Lattice(sf::RenderWindow&, int argc, char *argv[]);
};
