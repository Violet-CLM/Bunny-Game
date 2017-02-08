#include <array>
#include "stdafx.h"
#include "LatticeHooks.h"
#include "Lattice.h"
#include "Drawing.h"
#include "Misc.h"
#include "Constants.h"
#include "Shaders.h"

const ObjectList* Lattice::ObjectInitializationList = nullptr;

unsigned int Lattice::RenderFrame = 0;
unsigned int Lattice::GetFramesElapsed() { return RenderFrame; }

void Lattice::LoadLevel(std::wstring& Filepath)
{
	CurrentLevel = Level::LoadLevel(Filepath);
	if (CurrentLevel != nullptr)
		Window->setTitle(CurrentLevel->Name);
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
	CurrentLevel->Update(Keys);
	Keys.Update();
}

void Lattice::Render(double leftoverTimeElapsed)
{
	++RenderFrame;
	Window->clear();
	Window->draw(*CurrentLevel, SpriteMode::Paletted.GetShader());
	Window->display();
	
#ifdef SHOW_FPS
	static sf::Clock Clock;
	FPS = (int)(1000000ll / Clock.restart().asMicroseconds());
#endif
}

	

Lattice::Lattice(sf::RenderWindow& window, std::wstring& Filepath, const ObjectList& objectList) : Window(&window)
{
	Window = &window;
	ObjectInitializationList = &objectList;
	if (!Filepath.empty()) {
		LoadLevel(Filepath);
	}
		
	if (CurrentLevel == nullptr)
		return;

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
}

int main(int argc, char *argv[])
{
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS), "Error Loading Level", sf::Style::Titlebar | sf::Style::Close);
	window.setFramerateLimit(FPS_MAX); //solves all problems now and forever
	FullScreenQuad.setDimensions(WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS);

	if (!Hook_Init())
		return -1;
	const ObjectList& objectList = Hook_GetObjectList();
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

	Lattice Game(window, filename, objectList);

	//_CrtDumpMemoryLeaks();
	return 0;
}
