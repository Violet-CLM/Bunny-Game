#pragma once

#define AMMO_MAX 99

namespace Weapon {
	enum {
		Blaster, Bouncer, Ice, Seeker, RF, Toaster, TNT, Gun8, Gun9
		, LAST
	};
}

const int AmmoIconAnimIDs[Weapon::LAST] = {-1, 25, 29, 34, 49, 57, 59, 62, 68};