#include <array>
#include "stdafx.h"
#include "LatticeHooks.h"
#include "Lattice.h"
#include "Drawing.h"
#include "Misc.h"
#include "Constants.h"
#include "Shaders.h"

const ObjectList* Lattice::ObjectInitializationList = nullptr;
std::stack<std::unique_ptr<Stage>> Lattice::Stages;
std::unique_ptr<Stage> Lattice::StageToDelete(nullptr);
sf::RenderWindow* Lattice::Window;

unsigned int Lattice::RenderFrame = 0;
unsigned int Lattice::GetFramesElapsed() { return RenderFrame; }
#ifdef SHOW_FPS
	int Lattice::FPS;
#endif

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
	Stages.top()->Update(Keys);
	Keys.Update();
}

void Lattice::Render(double leftoverTimeElapsed)
{
	++RenderFrame;
	Window->clear();
	VideoBuffer.draw(*Stages.top(), Shaders[DefaultShaders::Paletted]);
	Hook_DrawToWindow(VideoBuffer, *Window);
	Window->display();
	
#ifdef SHOW_FPS
	static sf::Clock Clock;
	FPS = (int)(1000000ll / Clock.restart().asMicroseconds());
#endif
}


void Lattice::SetWindowTitle(const sf::String& title) { Window->setTitle(title); }

bool Lattice::Loop() {
	double previous = getCurrentTime();
	double lag = 0.0;
	while (!Stages.empty()) {
		const double current = getCurrentTime();
		const double elapsed = current - previous;
		previous = current;
		lag += elapsed;

		if (!ProcessInput()) //false: window closed
			break;

		while (lag >= MS_PER_UPDATE)
		{
			Update();
			if (StageToDelete) { //something in the Update call ended that Stage altogether
				StageToDelete.reset();
				return true; //break out of loop, to prevent skipping over a bunch of renderframes, but don't end the application yet
			}
			lag -= MS_PER_UPDATE;
		}
		Render(lag / MS_PER_UPDATE);
	}
	return false;
}
Lattice::Lattice(sf::RenderWindow& window, int argc, char *argv[]) {
	Window = &window;
	VideoBuffer.create(WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS);
	VideoBuffer.clear();

	Hook_DetermineInitialStage(Stages, argc, argv);

	while (Loop());

	//cleanup
	StageToDelete.reset();
	while (!Stages.empty())
		Stages.pop();
}

int main(int argc, char *argv[])
{
	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS), "Bunny Game", sf::Style::Titlebar | sf::Style::Close);
	window.setFramerateLimit(FPS_MAX); //solves all problems now and forever

	FullScreenQuad.setDimensions(WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS);
	for (int i = 0; i < 4; ++i)
		FullScreenQuad.vertices[i].texCoords = FullScreenQuad.vertices[3 - i].position; //vertically flipped
	FullScreenQuadNonFlipped.setDimensions(WINDOW_WIDTH_PIXELS, WINDOW_HEIGHT_PIXELS);
	for (int i = 0; i < 4; ++i)
		FullScreenQuadNonFlipped.vertices[i].texCoords = FullScreenQuadNonFlipped.vertices[i].position;

	if (!Hook_Init())
		return -1;
	Lattice::ObjectInitializationList = &Hook_GetObjectList();
	InitPopulateTextureArrays();
	InitCreateShaders(Shaders, ShaderSources);
	SpriteMode::Normal = SpriteMode(Shaders[DefaultShaders::Normal], 0);
	SpriteMode::Paletted = SpriteMode(Shaders[DefaultShaders::Paletted], 0);

	{ //set working directory to application's directory
		wchar_t applicationDirectory[MAX_PATH]; //http://stackoverflow.com/questions/143174/how-do-i-get-the-directory-that-a-program-is-running-from
		std::wstring applicationFilename(applicationDirectory, GetModuleFileName(NULL, applicationDirectory, MAX_PATH));
		_wchdir(applicationFilename.substr(applicationFilename.find_last_of(L"/\\")).c_str());
	}

	Lattice Game(window, argc, argv);

	//_CrtDumpMemoryLeaks();
	return 0;
}
