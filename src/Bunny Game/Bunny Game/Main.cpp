#include "Game.h"
#include "BunnyObjectList.h"

int main(int argc, char *argv[]) {
	bool isTSF = false; //todo, obviously

	Lattice game;
	game.SetDefaultAnimList(GetDefaultAnimList(isTSF));
	game.SetObjectActivityFunction(ShouldObjectsBeActive);
	game.SetObjectList(GetObjectList(isTSF));
	//game.SetObjectCollisionCallback(ObjectsShouldCollide);
	game.StartGame(argc, argv);
}