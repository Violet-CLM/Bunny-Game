#include <array>
#include "stdafx.h"
#include "Game.h"
#include "Drawing.h"
#include "Misc.h"
#include "Constants.h"
#include "Shaders.h"

int FPS;

void Lattice::LoadLevel(std::wstring& Filepath, PreloadedAnimationsList& anims, ObjectList& objs)
{
	Level = Level::LoadLevel(Filepath, anims, objs, SetupPaletteTables, PaletteLineCount);
	if (Level != nullptr)
		Window->setTitle(Level->Name);
}

Lattice::Lattice() {}

void Lattice::StartGame(sf::RenderWindow& window, std::wstring& Filepath)
{
	Window = &window;
	if (!Filepath.empty()) {
		LoadLevel(Filepath, DefaultAnimList, *ObjectInitializationList);
	}
}

bool Lattice::ProcessInput()
{
	sf::Event event;
	while (Window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed) {
			Window->close();
			return false;
		} else if (event.type == sf::Event::KeyPressed)
			Keys.SetKey(event.key.code, true);
		else if (event.type == sf::Event::KeyReleased)
			Keys.SetKey(event.key.code, false);
	}
	return true;
}

void Lattice::Update()
{
	Level->Update(*ActivateObjects, *CollideObjects, *UpdateHUD, Keys);
	Keys.Update();
}

void Lattice::Render(double leftoverTimeElapsed)
{
	Window->clear();
	Window->draw(*Level, SpriteMode::Paletted.GetShader());
	Window->display();
	
#ifdef SHOW_FPS
	static sf::Clock Clock;
	FPS = (int)(1000000ll / Clock.restart().asMicroseconds());
#endif
}

int Lattice::StartGame(int argc, char *argv[])
{
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS), "Error Loading Level", sf::Style::Titlebar | sf::Style::Close);
	window.setFramerateLimit(FPS_MAX); //solves all problems now and forever

	InitPopulateTextureArrays();
	InitCreateShaders(Shaders, ShaderSources);
	SpriteMode::Normal = SpriteMode(Shaders[DefaultShaders::Normal], 0);
	SpriteMode::Paletted = SpriteMode(Shaders[DefaultShaders::Paletted], 0);

	{ //set working directory to application's directory
		wchar_t applicationDirectory[MAX_PATH]; //http://stackoverflow.com/questions/143174/how-do-i-get-the-directory-that-a-program-is-running-from
		std::wstring applicationFilename(applicationDirectory, GetModuleFileName(NULL, applicationDirectory, MAX_PATH));
		_wchdir(applicationFilename.substr(applicationFilename.find_last_of(L"/\\")).c_str());
	}

	std::wstring filename = L"Diam3.j2l";
	if (argc == 2) {
		filename = WStringFromCharArray(argv[1]);
		if (!(filename.length() > 4 && filename.substr(filename.length() - 4, 4) == L".j2l"))
			filename += L".j2l";
	}
	StartGame(window, filename);
	if (Level == nullptr)
		return 0;

	double previous = getCurrentTime();
	double lag = 0.0;
	while (true)
	{
		const double current = getCurrentTime();
		const double elapsed = current - previous;
		previous = current;
		lag += elapsed;

		if (!ProcessInput()) //false: window closed
			break;

		while (lag >= MS_PER_UPDATE)
		{
			Update();
			lag -= MS_PER_UPDATE;
		}
		Render(lag / MS_PER_UPDATE);
	}

	//_CrtDumpMemoryLeaks();
	return 0;
}

void Lattice::DefineShaders(std::vector<std::string>& otherShaders, PaletteTableSetupFunction* palFunc, unsigned int lastPalLineID)
{
	ShaderSources.insert(ShaderSources.end(), otherShaders.begin(), otherShaders.end());
	PaletteLineCount = lastPalLineID;
	SetupPaletteTables = palFunc;
}

void Lattice::SetDefaultAnimList(PreloadedAnimationsList& d)
{
	DefaultAnimList = d;
}

void Lattice::SetObjectActivityFunction(ObjectActivityFunction& a)
{
	ActivateObjects = &a;
}

void Lattice::SetObjectCollisionTestFunction(ObjectCollisionTestFunction& a)
{
	CollideObjects = &a;
}
void Lattice::SetHUDUpdateFunction(HUDUpdateFunction& a)
{
	UpdateHUD = &a;
}

void Lattice::SetObjectList(ObjectList* ol)
{
	ObjectInitializationList = ol;
}
