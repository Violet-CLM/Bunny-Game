#include "Particles.h"
#include "Layer.h"
#include "BunnyMisc.h"
#include "CharStrings.h"
void Particle::Behave() {
	const auto gravity = 0.125f;
	const static std::vector<sf::Vector2i> FireDrawLocations[] = {
		{ { 0,0 } },
		{ { 1,0 } },
		{ { 0,1 },{ 1,1 } },
		{ { 2,0 },{ -1,1 },{ 2,1 },{ 3,1 },{ 0,2 },{ 1,2 },{ 2,2 } }
	};

	switch (ParticleType(particleType)) {
	case ParticleType::tSpark:
		if (!(RandFac(3)))
			if ((Spark.color += Spark.colorDelta) == Spark.colorStop)
				break;
		//if (yPos <= GameGlobals->level.waterLevel) //todo water
		SpeedY += gravity / 8;
		//else
		//	SpeedY += gravity / 32;
		PositionX += SpeedX;
		PositionY += SpeedY;
		{ //draw
			float xDrawPixel = PositionX, yDrawPixel = PositionY;
			for (int i = 0; i < 16; ++i, xDrawPixel -= SpeedX, yDrawPixel -= SpeedY)
				LayerPtr->AppendPixel(int(xDrawPixel), int(yDrawPixel), Spark.color);
		}
		return;
	case ParticleType::tIceTrail:
		if (!(RandFac(7)))
			if ((IceTrail.color += IceTrail.colorDelta) == IceTrail.colorStop)
				break;
		//if (PositionY <= GameGlobals->level.waterLevel) //todo water
			SpeedY += gravity / 4;
		//else
		//	SpeedY += gravity / 16;
		PositionY += SpeedY;
		//PositionX += SpeedX; //not used
		{ //draw
			LayerPtr->AppendPixel(int(PositionX), int(PositionY), IceTrail.color);
			LayerPtr->AppendPixel(int(PositionX) + RandFac(1), int(PositionY) + RandFac(1), IceTrail.color);
		}
		return;
	case ParticleType::tScore:
		{
			auto speed = SpeedX;
			PositionX += SpeedX;
			SpeedX += speed / 16;
			speed = SpeedY;
			PositionY += SpeedY;
			SpeedY += speed / 16;
		}
		WriteText(GetWriteCharacterFunction(*LayerPtr), int(PositionX),int(PositionY), Score.text, *AnimationPtr->AnimFrames);
		return;
	case ParticleType::tPixel:
		{ //x movement
			ApproachZeroByUnit(SpeedX, 0.00390625f);
			const auto prospectiveXPos = PositionX + SpeedX;
			if (LayerPtr->MaskedPixel(int(prospectiveXPos), int(PositionY))) //wall to side
				SpeedX = -7 * SpeedX / 8; //rebound
			else PositionX = prospectiveXPos;
		}
		PositionY += SpeedY;
		//if (PositionY <= GameGlobals->level.waterLevel)
			SpeedY += gravity / 2;
		//else
		//	SpeedY += gravity / 8;
		if (LayerPtr->MaskedPixel(int(PositionX), int(PositionY))) {
			SpeedY /= -2;
			if (abs(SpeedY) < 0.25f)
				break; //done
			else PositionY += SpeedY; //extract from floor/ceiling=
		}
		{ //draw
			const auto* src = Pixel.color;
			for (int x = 0; x < size; ++x)
				for (int y = 0; y < size; ++y)
					LayerPtr->AppendPixel(int(PositionX) + x, int(PositionY) + y, *src++);
		}
		return;
	case ParticleType::tFire:
		{
			const auto r = RandFac(63);
			if (!(r & 7)) {
				if (!r)
					AddSmoke(*LayerPtr, sf::Vector2f(PositionX, PositionY - 1));
				if ((Fire.color += Fire.colorDelta) == Fire.colorStop)
					break;
			}
		}
		//if (PositionY <= GameGlobals->level.waterLevel)
			SpeedY += gravity / 8;
		//else
		//	SpeedY += gravity / 32;
		PositionY += SpeedY;
		if (LayerPtr->MaskedPixel(int(PositionX), int(PositionY))) { //bounce
			PositionY -= SpeedY;
			SpeedY /= -2;
		}
		{ //draw
			for (unsigned int i = 0; i < size; ++i)
				for (const auto& it : FireDrawLocations[i])
					LayerPtr->AppendPixel(int(PositionX) + it.x, int(PositionY) + it.y, Fire.color);
		}
		return;
	case ParticleType::tSmoke:
		if (!RandFac(0xF))
			if (--Smoke.lifetime < 64)
				break;
		PositionY += -2 * int(RandFac(0x7FFF)) / 65536.f;
		PositionX += sintable(RandFac(1023));
		{ //draw
			unsigned int size = (Smoke.lifetime & 3) + 1;
			while (size--)
				for (const auto& it : FireDrawLocations[size]) //also serves as SmokeDrawLocations I guess
					LayerPtr->AppendPixel(int(PositionX) + it.x, int(PositionY) + it.y, Smoke.lifetime);
		}
		return;
	default:
		return;
	}
	particleType = ParticleType::tInactive;
}
Particle* Particle::Add(Layer& layer) {
	auto result = std::find_if(layer.Particles.begin(), layer.Particles.end(), [](Particle const& particle)
	{
		return particle.particleType == Particle::ParticleType::tInactive;
	});
	if (result != layer.Particles.end()) {
		result->LayerPtr = &layer;
		return &*result;
	}
	return nullptr;
}
Particle* Particle::Add(Layer& layer, ParticleType type, const sf::Vector2f& position) {
	auto result = Add(layer);
	if (result != nullptr) {
		result->particleType = type;
		result->PositionX = position.x;
		result->PositionY = position.y;
	}
	return result;
}

Particle* Particle::AddSpark(Layer& layer, const sf::Vector2f& position, int DirectionX) {
	auto result = Add(layer, ParticleType::tSpark, position);
	if (result != nullptr) {
		result->SpeedX = (DirectionX << 1) * int(RandFac(0x3FFF) + 0x8000) / 65536.f;
		result->SpeedY = int(0x2000 - RandFac(0x7FFF)) / 65536.f;
		result->Spark.color = 40;
		result->Spark.colorDelta = 1;
		result->Spark.colorStop = 46;
	}
	return result;
}

Particle* Particle::AddIceTrail(Layer& layer, const sf::Vector2f& position) {
	auto result = Add(layer, ParticleType::tIceTrail, position);
	if (result != nullptr) {
		result->SpeedY = -((int)RandFac(0x7FFF)) / 65536.f;
		result->IceTrail.color = 32;
		result->IceTrail.colorDelta = 1;
		result->IceTrail.colorStop = 40;
	}
	return result;
}

Particle* Particle::AddScore(Layer& layer, const sf::Vector2f& position, int score, const Animation* const animPtr) {
	auto result = Add(layer, ParticleType::tScore, position);
	if (result != nullptr) {
		result->SpeedX = (-32768 - int(RandFac(0x3FFF))) / 65536.f;
		result->SpeedY = (-65536 - int(RandFac(0x7FFF))) / 65536.f;
		result->AnimationPtr = animPtr;
		sprintf_s(result->Score.text, "%d", score);
	}
	return result;
}

Particle* Particle::AddPixel(Layer& layer, const sf::Vector2f& position, int size) {
	auto result = Add(layer, ParticleType::tPixel, position);
	if (result != nullptr) {
		if (size < 0) size = RandFac(3);
		if (unsigned(size) > 2) size = 2;
		result->size = size + 1; //1, 2, or 3
	}
	return result;
}

Particle* Particle::AddFire(Layer& layer, const sf::Vector2f& position, sf::Uint8 startColor) {
	auto result = Add(layer, ParticleType::tFire, position);
	if (result != nullptr) {
		result->Fire.color = startColor;
		result->Fire.colorStop = (startColor & ~7) + 8;
		result->Fire.colorDelta = 1;
	}
	return result;
}

Particle* Particle::AddSmoke(Layer& layer, const sf::Vector2f& position) {
	auto result = Add(layer, ParticleType::tSmoke, position);
	if (result != nullptr) {
		result->Smoke.lifetime = 71;
	}
	return result;
}
