#include "Bunny.h"


Bunny::Bunny(ObjectStartPos & objStart) : BunnyObject(objStart)
{
	AnimID = 67;
	DetermineFrame(1);
}
//void Bunny::Draw(Layer *) const
//{
//}

void Bunny::ProcessInput(GameState& gameState) {
	//todo base all these codes on Bunny2.CFG
	KeyDown = gameState.Keys.Key(sf::Keyboard::Down);
	KeyUp = !KeyDown && gameState.Keys.Key(sf::Keyboard::Up);
	KeyRight = gameState.Keys.Key(sf::Keyboard::Right);
	KeyLeft = !KeyRight && gameState.Keys.Key(sf::Keyboard::Left);
	KeyFire = gameState.Keys.Key(sf::Keyboard::Space);
	KeySelect = gameState.Keys.Key(sf::Keyboard::Return);
	KeyRun = gameState.Keys.Key(sf::Keyboard::LShift) || gameState.Keys.Key(sf::Keyboard::RShift);
	KeyJump = gameState.Keys.Key(sf::Keyboard::LControl) || gameState.Keys.Key(sf::Keyboard::RControl);
}
void Bunny::Behave(GameState& gameState)
{
	ProcessInput(gameState);

	const float movementSpeed = KeyRun ? 2 : 1;
	if (KeyDown)
		PositionY += movementSpeed;
	else if (KeyUp)
		PositionY -= movementSpeed;
	if (KeyLeft)
		PositionX -= movementSpeed;
	else if (KeyRight)
		PositionX += movementSpeed;

	gameState.CenterCamera(PositionX, PositionY);
}
