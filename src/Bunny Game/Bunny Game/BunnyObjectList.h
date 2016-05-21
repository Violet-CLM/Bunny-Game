#pragma once
#include <array>
#include <map>
#include <set>
#include "SFML/Config.hpp"
#include "Objects.h"
#include "Constants.h"

enum AnimSets {
	Ammo, Bat, BeeBoy, Bees, BigBox, BigRock, BigTree, BilsBoss, Bird, Bird3D, BollPlat, Bonus, Boss, Bridge, Bubba, BumBee, Butterfly,
	CarrotPole, Cat, Cat2, Caterpil, Chuck, Common, Continue, Demon, DestScen, Devan, DevilDevan, DiamPole, Dog, Door, DragFly, Dragon,
	Ending, EndTuneJazz, EndTuneLori, EndTuneSpaz, EpicLogo, Eva, Faces, Fan, FatChk, Fencer, Fish, Flag, Flare, Font, Frog, FruitPlat,
	GemRing, Glove, GrassPlat, Hatter, Helmut, Intro, Jazz, Jazz3D, JazzSounds, JunglePole, Labrat, Lizard, Lori, Lori2, LoriSounds,
	Menu, MenuFont, MenuSounds, Monkey, Moth, Orange, P2, Pickups, Pinball, PinkPlat, PsychPole, Queen, Rapier, Raven, Robot, Rock, RockTurt, Rush,
	Science, Skeleton, SmalTree, Snow, SoncShip, SonicPlat, Spark, Spaz, Spaz2, Spaz3D, SpazSounds, SpikeBoll, SpikeBoll3D, SpikePlat, Spring, Steam, Stoned, Sucker,
	TubeTurt, TufBoss, TufTurt, Turtle, Tweedle, Uterus, Vine, Warp10, Warp100, Warp20, Warp50, Wind, Witch, XBilsy, XLizard, XTurtle, ZDog, ZSpark


	, LAST
};

enum EventIDs {
	PLAYERBULLET1 = 1,		ONEWAY = 1,
	PLAYERBULLET2 = 2,		HURT = 2,
	PLAYERBULLET3 = 3,		VINE = 3,
	PLAYERBULLET4 = 4,		HOOK = 4,
	PLAYERBULLET5 = 5,		SLIDE = 5,
	PLAYERBULLET6 = 6,		HPOLE = 6,
	PLAYERBULLET8 = 7,		VPOLE = 7,
	PLAYERBULLET9 = 8,		FLYOFF = 8,
	PLAYERBULLETP1 = 9,		RICOCHET = 9,
	PLAYERBULLETP2 = 10,	BELTRIGHT = 10,
	PLAYERBULLETP3 = 11,	BELTLEFT = 11,
	PLAYERBULLETP4 = 12,	BELTACCRIGHT = 12,
	PLAYERBULLETP5 = 13,	BELTACCLEFT = 13,
	PLAYERBULLETP6 = 14,	STOPENEMY = 14,
	PLAYERBULLETP8 = 15,	WINDLEFT = 15,
	PLAYERBULLETP9 = 16,	WINDRIGHT = 16,
	PLAYERBULLETC1 = 17,	ENDOFLEVEL = 17,
	PLAYERBULLETC2 = 18,	WARPEOL = 18,
	PLAYERBULLETC3 = 19,	REVERTMORPH = 19,
	BULLET = 20,			FLOATUP = 20,
	CATSMOKE = 21,			TRIGGERROCK = 21,
	SHARD = 22,				DIMLIGHT = 22,
	EXPLOSION = 23,			SETLIGHT = 23,
	BOUNCEONCE = 24,		LIMITXSCROLL = 24,
	REDGEMTEMP = 25,		RESETLIGHT = 25,
	PLAYERLASER = 26,		WARPSECRET = 26,
	UTERUSEL = 27,			ECHO = 27,
	BIRD = 28,				ACTIVATEBOSS = 28,
	BUBBLE = 29,			JAZZSTART = 29,
							SPAZSTART = 30,
							MPSTART = 31,
							LORISTART = 32,
	GUN3AMMO3 = 33,
	GUN2AMMO3 = 34,
	GUN4AMMO3 = 35,
	GUN5AMMO3 = 36,
	GUN6AMMO3 = 37,
	GUN7AMMO3 = 38,
	GUN8AMMO3 = 39,
	GUN9AMMO3 = 40,
	TURTLESHELL = 41,
	SWINGVINE = 42,
	BOMB = 43,
	SILVERCOIN = 44,
	GOLDCOIN = 45,
	GUNCRATE = 46,
	CARROTCRATE = 47,
	ONEUPCRATE = 48,
	GEMBARREL = 49,
	CARROTBARREL = 50,
	ONEUPBARREL = 51,
	BOMBCRATE = 52,
	GUN3AMMO15 = 53,
	GUN2AMMO15 = 54,
	GUN4AMMO15 = 55,
	GUN5AMMO15 = 56,
	GUN6AMMO15 = 57,
	TNT = 58,
	AIRBOARDGENERATOR = 59,
	FROZENGREENSPRING = 60,
	GUNFASTFIRE = 61,
	SPRINGCRATE = 62,
	REDGEM = 63,
	GREENGEM = 64,
	BLUEGEM = 65,
	PURPLEGEM = 66,
	SUPERREDGEM = 67,
	BIRDCAGE = 68,
	GUNBARREL = 69,
	GEMCRATE = 70,
	MORPHMONITOR = 71,
	ENERGYUP = 72,
	FULLENERGY = 73,
	FIRESHIELD = 74,
	WATERSHIELD = 75,
	LIGHTSHIELD = 76,
	MAXWEAPON = 77,
	AUTOFIRE = 78,
	FASTFEET = 79,
	EXTRALIFE = 80,
	ENDOFLEVELPOST = 81,
	SAVEPOST = 83,
	BONUSLEVELPOST = 84,
	REDSPRING = 85,
	GREENSPRING = 86,
	BLUESPRING = 87,
	INVINCIBILITY = 88,
	EXTRATIME = 89,
	FREEZER = 90,
	HREDSPRING = 91,
	HGREENSPRING = 92,
	HBLUESPRING = 93,
	BIRDMORPHMONITOR = 94,
	TRIGGERCRATE = 95,
	FLYCARROT = 96,
	RECTREDGEM = 97,
	RECTGREENGEM = 98,
	RECTBLUEGEM = 99,
	TUFTURT = 100,
	TUFBOSS = 101,
	LABRAT = 102,
	DRAGON = 103,
	LIZARD = 104,
	BUMBEE = 105,
	RAPIER = 106,
	SPARK = 107,
	BAT = 108,
	SUCKER = 109,
	CATERPILLAR = 110,
	CHESHIRE1 = 111,
	CHESHIRE2 = 112,
	HATTER = 113,
	BILSYBOSS = 114,
	SKELETON = 115,
	DOGGYDOGG = 116,
	NORMTURTLE = 117,
	HELMUT = 118,
	DEMON = 120,
	DRAGONFLY = 123,
	MONKEY = 124,
	FATCHK = 125,
	FENCER = 126,
	FISH = 127,
	MOTH = 128,
	STEAM = 129,
	ROCK = 130,
	GUN1POWER = 131,
	GUN2POWER = 132,
	GUN3POWER = 133,
	GUN4POWER = 134,
	GUN5POWER = 135,
	GUN6POWER = 136,
	PINLEFTPADDLE = 137,
	PINRIGHTPADDLE = 138,
	PIN500BUMP = 139,
	PINCARROTBUMP = 140,
	APPLE = 141,
	BANANA = 142,
	CHERRY = 143,
	ORANGE = 144,
	PEAR = 145,
	PRETZEL = 146,
	STRAWBERRY = 147,
	STEADYLIGHT = 148,
	PULZELIGHT = 149,
	FLICKERLIGHT = 150,
	QUEENBOSS = 151,
	FLOATSUCKER = 152,
	BRIDGE = 153,
	LEMON = 154,
	LIME = 155,
	THING = 156,
	WMELON = 157,
	PEACH = 158,
	GRAPES = 159,
	LETTUCE = 160,
	EGGPLANT = 161,
	CUCUMB = 162,
	COKE = 163,
	PEPSI = 164,
	MILK = 165,
	PIE = 166,
	CAKE = 167,
	DONUT = 168,
	CUPCAKE = 169,
	CHIPS = 170,
	CANDY = 171,
	CHOCBAR = 172,
	ICECREAM = 173,
	BURGER = 174,
	PIZZA = 175,
	FRIES = 176,
	CHICKLEG = 177,
	SANDWICH = 178,
	TACOBELL = 179,
	WEENIE = 180,
	HAM = 181,
	CHEESE = 182,
	FLOATLIZARD = 183,
	STANDMONKEY = 184,
	DESTRUCTSCENERY = 185,
	DESTRUCTSCENERYBOMB = 186,
	COLLAPSESCENERY = 187,
	STOMPSCENERY = 188,
	GEMSTOMP = 189,
	RAVEN = 190,
	TUBETURTLE = 191,
	GEMRING = 192,
	ROTSMALLTREE = 193,
	AMBIENTSOUND = 194,
	UTERUS = 195,
	CRAB = 196,
	WITCH = 197,
	ROCKTURT = 198,
	BUBBA = 199,
	DEVILDEVAN = 200,
	DEVANROBOT = 201,
	ROBOT = 202,
	CARROTUSPOLE = 203,
	PSYCHPOLE = 204,
	DIAMONDUSPOLE = 205,
							TEXT = 207,
							WATERLEVEL = 208,
	FRUITPLATFORM = 209,
	BOLLPLATFORM = 210,
	GRASSPLATFORM = 211,
	PINKPLATFORM = 212,
	SONICPLATFORM = 213,
	SPIKEPLATFORM = 214,
	SPIKEBOLL = 215,
	GENERATOR = 216,
	EVA = 217,
	BUBBLER = 218,
	TNTPOWER = 219,
	GUN8POWER = 220,
	GUN9POWER = 221,
							FROGMORPH = 222,
	SPIKEBOLL3D = 223,
	SPRINGCORD = 224,
	BEES = 225,
	COPTER = 226,
	LASERSHIELD = 227,
	STOPWATCH = 228,
	JUNGLEPOLE = 229,
	WARP = 230,
	BIGROCK = 231,
	BIGBOX = 232,
	TRIGGERSCENERY = 234,
	SONICBOSS = 235,
	BUTTERFLY = 236,
	BEEBOY = 237,
	SNOW = 238,
							WARPTARGET = 240,
	TWEEDLEBOSS = 241,
							PATH = 242,
	AIRBOARD = 243,
	FLAG = 244,
							PLAYERVARIABLEZONE = 245,
							TRIGGERZONE = 246,
	XNORMTURTLE = 248,
	XLIZARD = 249,
	XFLOATLIZARD = 250,
	XBILSYBOSS = 251,
	ZCAT = 252,
	ZGHOST = 253
};

ObjectList* GetObjectList(bool);
PreloadedAnimationsList GetDefaultAnimList(bool);
bool ObjectsShouldCollide(const GameObject&, const GameObject&);
void ShouldObjectsBeActive(Level&);