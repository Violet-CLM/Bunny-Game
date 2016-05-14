#include "stdafx.h"
#include "Game.h"
#include "Drawing.h"
#include "Misc.h"
#include "Constants.h"
#include "Shaders.h"

void Lattice::LoadLevel(std::wstring& Filepath, PreloadedAnimationsList& anims, ObjectList& objs)
{
	Level = Level::LoadLevel(Filepath, anims, objs);
	if (Level != nullptr)
		Window->setTitle(Level->Name);
}

Lattice::Lattice() {}

void Lattice::StartGame(sf::RenderWindow& window, std::wstring& Filepath)
{
	Window = &window;
	if (!Filepath.empty())
		LoadLevel(Filepath, DefaultAnimList, *ObjectInitializationList);
}

void Lattice::ProcessInput()
{
	sf::Event event;
	while (Window->pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			Window->close();
	}
}

void Lattice::Update()
{
	Level->Update(sf::Mouse::getPosition(*Window), *ActivateObjects);
}

void Lattice::Render(double leftoverTimeElapsed)
{
	Window->clear();
	Window->draw(*Level, sf::RenderStates(shaders[shader_NORMAL]));
	Window->display();
}

int Lattice::StartGame(int argc, char *argv[])
{
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS), "Error Loading Level", sf::Style::Titlebar | sf::Style::Close);
	window.setFramerateLimit(60); //solves all problems now and forever

	InitPopulateTextureArrays();
	InitCreateShaders();
	std::wstring filename = L"Diam3.j2l";// L"C:\\Games\\Jazz2\\Diam3.j2l";
	if (argc == 2) {
		std::wstring proposedFilename = WStringFromCharArray(argv[1]);
		if (!(proposedFilename.length() > 4 && proposedFilename.substr(filename.length() - 5, 4) == L".j2l"))
			proposedFilename += L".j2l";
		filename = proposedFilename;
	}
	StartGame(window, filename);
	if (Level == nullptr)
		return 0;

	double previous = getCurrentTime();
	double lag = 0.0;
	while (window.isOpen())
	{
		const double current = getCurrentTime();
		const double elapsed = current - previous;
		previous = current;
		lag += elapsed;

		ProcessInput();
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

void Lattice::SetDefaultAnimList(PreloadedAnimationsList& d)
{
	DefaultAnimList = d;
}

void Lattice::SetObjectActivityFunction(ObjectActivityFunction& a)
{
	ActivateObjects = &a;
}

void Lattice::SetObjectList(ObjectList* ol)
{
	ObjectInitializationList = ol;
}
