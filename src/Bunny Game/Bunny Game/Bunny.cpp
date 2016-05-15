#include "Bunny.h"


Bunny::Bunny(ObjectStartPos & objStart) : BunnyObject(objStart)
{
	AnimID = 67;
	DetermineFrame(1);
	
}
//void Bunny::Draw(Layer *) const
//{
//}

void Bunny::GetInput(const KeyStates& keys) {
	//todo base all these codes on JAZZ2.CFG
	KeyDown = keys.Key(sf::Keyboard::Down);
	KeyUp = !KeyDown && keys.Key(sf::Keyboard::Up);
	KeyRight = keys.Key(sf::Keyboard::Right);
	KeyLeft = !KeyRight && keys.Key(sf::Keyboard::Left);
	KeyFire = keys.Key(sf::Keyboard::Space);
	KeySelect = keys.Key(sf::Keyboard::Return);
	KeyRun = keys.Key(sf::Keyboard::LShift) || keys.Key(sf::Keyboard::RShift);
	KeyJump = keys.Key(sf::Keyboard::LControl) || keys.Key(sf::Keyboard::RControl);
}
void Bunny::ProcessInput()
{
	const float movementSpeed = KeyRun ? 2.f : 1.f;
	if (KeyDown)
		PositionY += movementSpeed;
	else if (KeyUp)
		PositionY -= movementSpeed;
	if (KeyLeft)
		PositionX -= movementSpeed;
	else if (KeyRight)
		PositionX += movementSpeed;
}
void Bunny::DoLandscapeCollision()
{
}
void Bunny::DoZoneDetection()
{
}
void Bunny::ProcessAction()
{
}
void Bunny::AdjustViewpoint(GameState& gameState) const
{
	gameState.CenterCamera(PositionX, PositionY);
}
void Bunny::Behave(GameState& gameState)
{
	GetInput(gameState.Keys);
	ProcessInput();
	DoLandscapeCollision();
	DoZoneDetection();
	ProcessAction();
	AdjustViewpoint(gameState);
}
