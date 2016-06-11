#include "Game.h"
#include "BunnyObjectList.h"
#include "BunnyVersionDependentStuff.h"
#include "BunnyShaders.h"
#include "Windows.h"
#include "Misc.h"

int main(int argc, char *argv[]) {
	bool isTSF;
	if (!IsTSF(isTSF))
		return -1;

	Lattice game;
	InitializeRabbitAnimIDs(isTSF);
	game.DefineShaders(BunnyShaderSources, GeneratePaletteTextureBunnyEdition, BunnyPaletteLineNames::LAST);
	game.SetDefaultAnimList(GetDefaultAnimList(isTSF));
	game.SetObjectActivityFunction(ShouldObjectsBeActive);
	game.SetObjectList(GetObjectList(isTSF));
	//game.SetObjectCollisionCallback(ObjectsShouldCollide);
	return game.StartGame(argc, argv);
}