#pragma once
#include "SFML/Config.hpp"
#include "SFML/Graphics.hpp"

struct Layer;
class Animation;

struct Particle {
	float PositionX, PositionY, SpeedX, SpeedY;
	sf::Uint8 particleType;
	sf::Uint8 size;
	bool active;
	enum ParticleType {
		tInactive, tPixel, tFire, tSmoke, tIceTrail, tSpark, tScore, tSnow, tTile
	};
	union {
		struct {
			sf::Uint8 color[9];
		} Pixel;
		struct {
			sf::Uint8 color; //40. increases, and drops drawing when it hits colorStop
			sf::Uint8 colorStop; //48
			char colorDelta; //1
			//sf::Uint8 padding[6];
		} Fire; //particleType 2; ignores xSpeed
		struct {
			sf::Uint8 lifetime; //23. decreases, and stops drawing when it hits 64
			//sf::Uint8 padding[8];
		} Smoke; //particleType 3; ignores xSpeed, ySpeed
		struct {
			sf::Uint8 color; //32. increases, and drops drawing when it hits colorStop
			sf::Uint8 colorStop; //40
			char colorDelta; //1
			//sf::Uint8 padding[6];
		} IceTrail; //particleType 4; ignores xSpeed
		struct {
			sf::Uint8 color; //40. increases, and drops drawing when it hits colorStop
			sf::Uint8 colorStop; //46
			char colorDelta; //1
			//sf::Uint8 padding[6];
		} Spark; //particleType 5
		struct {
			char text[9];
		} Score;
		struct {
			sf::Uint8 frame; //23. 0-7
			sf::Uint8 deathcounter; //24. once != 0, counts up to 70 and then disappears
			sf::Uint8 nocliptime; //25. counts down to 0, at which point masks are checked for
			//sf::Uint8 padding[6];
		} Snow; //particleType 7
		/*struct {
			sf::Uint8 frame; //23. 0-7 while falling, then 8-17 while splashing (and resized)
			sf::Uint8 padding[8];
		} Rain;
		struct {
			sf::Uint8 color; //23. 16 by default
			sf::Uint8 angle; //24. 0-255
			char angularSpeed; //25.
			sf::Uint8 petals; //26. 5 by default
			sf::Uint8 padding[5];
		} Flower;
		struct {
			sf::Uint8 frame; //23. 0-31
			sf::Uint8 deathcounter; //24. once != 0, counts up to 140 and then disappears
			bool noclip; //25.
			sf::Uint8 height; //26. if noclip is off, how far the leaf must be from the ground before it stops falling
			sf::Uint8 padding[5];
		} Leaf;
		struct {
			sf::Uint8 color; //23. 40 by default
			sf::Uint8 angle; //24.
			char angularSpeed; //25.
			sf::Uint8 frame; //26. 0 or 1
			sf::Uint8 colorChangeCounter; //27.
			sf::Uint8 colorChangeInterval; //28. 0 for constant color (default)
			sf::Uint8 padding[3];
		} Star;*/
		struct {
			sf::Uint8 quadrant;
			//sf::Uint8 padding[8];
		} Tile;
		struct {
			sf::Uint8 padding[9];
		} GENERIC;
	};

	const Animation* AnimationPtr;
	Layer* LayerPtr;

	void Behave();

private:
	friend class ElectroBlasterBullet;
	static Particle* Add(Layer&);
	static Particle* Add(Layer&, ParticleType, const sf::Vector2f&);
public:
	static Particle* AddSpark(Layer&, const sf::Vector2f&, int);
	static Particle* AddIceTrail(Layer&, const sf::Vector2f&);
	static Particle* AddScore(Layer&, const sf::Vector2f&, int, const Animation* const);
	static Particle* AddPixel(Layer&, const sf::Vector2f&, int = -1);
	static Particle* AddFire(Layer&, const sf::Vector2f&, sf::Uint8);
	static Particle* AddSmoke(Layer&, const sf::Vector2f&);
};
static_assert(sizeof(Particle) == 28 + sizeof(Animation*) + sizeof(Layer*), "Particle incorrect size!");
