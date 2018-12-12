#include "Particles.h"
#include "Layer.h"
#include "BunnyMisc.h"
#include "CharStrings.h"
void Particle::Behave() {
	const auto gravity = 0.125f;
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
		//if (particle->yPos <= GameGlobals->level.waterLevel) //todo water
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
