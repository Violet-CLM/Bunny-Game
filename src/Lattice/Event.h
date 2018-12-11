#pragma once
#include "SFML/Config.hpp"
#include "Constants.h"

struct Event {
private:
	sf::Uint32 raw;

public:
	EventID GetID() const;
	EventID SetID(EventID n);
	int GetDifficulty() const;
	int SetDifficulty(int f);
	bool GetActive() const;
	bool SetActive(bool f);
	bool GetIlluminated() const;
	bool SetIlluminated(bool f);
	__declspec(property(get = GetID, put = SetID)) EventID ID;
	__declspec(property(get = GetDifficulty, put = SetDifficulty)) int Difficulty;
	__declspec(property(get = GetActive, put = SetActive)) bool Active;
	__declspec(property(get = GetIlluminated, put = SetIlluminated)) bool Illuminated;

	int GetParameter(int, int) const;
	int SetParameter(int, int, int);

	sf::Uint32 operator=(sf::Uint32);
};
static_assert(sizeof(Event) == sizeof(sf::Uint32), "Event incorrect size!");