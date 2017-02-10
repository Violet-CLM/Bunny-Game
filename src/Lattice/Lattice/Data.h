#pragma once
#include <vector>
#include <initializer_list>
#include "SFML/Config.hpp"

typedef std::vector<sf::Uint8> DataTarget;

void LoadDataFromFile(const std::wstring& filename, std::initializer_list<const char*> dataNames, std::initializer_list<DataTarget*> dataTargets);
