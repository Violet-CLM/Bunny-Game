#pragma once
#include <set>
#include <map>
#include "SFML/Config.hpp"
#include "SFML/Graphics.hpp"
#include "Constants.h"

class Level;
struct Layer;
class GameObject;
class VertexCollectionQueue;

//Functions declared in this file are called by Lattice but are not defined by Lattice files, so they must be defined somewhere in the game files in order for the project to compile.

bool Hook_Init();
void Hook_InitAfterShadersConstructed();
void Hook_SetupPaletteTables(sf::Texture&, const sf::Color* const, std::array<sf::Color, COLORSPERPALETTE>&);
void Hook_ActivateObjects(Level&);
const ObjectList& Hook_GetObjectList();
bool Hook_CollideObjects(const GameObject&, const GameObject&);
void Hook_ClearSpriteQueues();
void Hook_LevelMain(Level&, unsigned int);
void Hook_GetAnimationList(Level&, PreloadedAnimationsList&);
void Hook_LevelLoad(Level&);
bool Hook_ShouldTexturedLayerBeUpdated(unsigned int);
bool Hook_ShouldTexturedLayerBeRendered(const Layer&, sf::RenderTarget&, sf::RenderStates, unsigned int);
void Hook_DrawToWindow(sf::RenderTexture&, sf::RenderWindow&);
