#pragma once

#define WEAPON_COUNT 9
#define AMMO_MAX 99

namespace Weapon {
	enum {
		Blaster, Bouncer, Ice, Seeker, RF, Toaster, TNT, Gun8, Gun9
	};
}

const int AmmoIconAnimIDs[WEAPON_COUNT] = {-1, 25, 29, 34, 49, 57, 59, 62, 68};