#include "Game.h"
#include "BunnyHUD.h"
#include "BunnyObjectList.h"
#include "BunnyVersionDependentStuff.h"
#include "BunnyShaders.h"
#include "Windows.h"
#include "Misc.h"

int main(int argc, char *argv[]) {
	if (!IsTSF(VersionTSF))
		return -1;

	Lattice game;
	InitializeRabbitAnimIDs();
	game.DefineShaders(BunnyShaderSources, GeneratePaletteTextureBunnyEdition, BunnyPaletteLineNames::LAST);
	game.SetDefaultAnimList(GetDefaultAnimList());
	game.SetObjectActivityFunction(ShouldObjectsBeActive);
	game.SetObjectList(GetObjectList());
	game.SetObjectCollisionTestFunction(ObjectsShouldCollide);
	game.SetHUDUpdateFunction(DrawPlayerHUD);
	return game.StartGame(argc, argv);
}