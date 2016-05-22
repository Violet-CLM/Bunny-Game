#include "Game.h"
#include "BunnyObjectList.h"
#include "Windows.h"
#include "Misc.h"

int main(int argc, char *argv[]) {
	bool isTSF;

	std::ifstream file("Anims.j2a", std::ios_base::binary);
	file.seekg(24, std::ios_base::beg);
	int animSetCount; file.read((char*)&animSetCount, sizeof(animSetCount));
	file.close();
	if (animSetCount == 109)
		isTSF = false;
	else if (animSetCount == 118)
		isTSF = true;
	else {
		ShowErrorMessage(L"Invalid Anims.j2a file");
		return -1;
	}

	Lattice game;
	game.SetDefaultAnimList(GetDefaultAnimList(isTSF));
	game.SetObjectActivityFunction(ShouldObjectsBeActive);
	game.SetObjectList(GetObjectList(isTSF));
	//game.SetObjectCollisionCallback(ObjectsShouldCollide);
	return game.StartGame(argc, argv);
}