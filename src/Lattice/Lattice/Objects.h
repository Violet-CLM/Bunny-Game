#pragma once
#include <memory>
#include <functional>
#include "Resources.h"
#include "Layer.h"

class ObjectStartPos {
	friend class GameObject;
private:
	float OriginX, OriginY;
	const AnimSet* Set;
public:
	ObjectStartPos(float X, float Y, const AnimSet* S) : OriginX(X), OriginY(Y), Set(S) {}
};

class GameObject {
	friend class ObjectInitialization;

public:
	float PositionX, PositionY;

	bool CollidesWith(const GameObject&) const;
	AnimFrame& GetFrame() const;

	GameObject(ObjectStartPos& objStart) {
		PositionX = OriginX = objStart.OriginX;
		PositionY = OriginY = objStart.OriginY;
		Set = objStart.Set;
	}

	float OriginX, OriginY;
private:

	unsigned int RadiusX, RadiusY;
	bool RoundedCorners;

	const AnimSet* Set;
public:
	unsigned int AnimID, FrameID;
	virtual void Behave(Level&) = 0;
	virtual void Draw(Layer*) const = 0;
protected:
	void DetermineFrame(int);
};

typedef GameObject* (*ObjectInitializationFunc)(ObjectStartPos&);

class Level;
class ObjectInitialization {
public:
	int AnimSetID;
	ObjectInitializationFunc Function;
	bool CreateObjectFromEventMap;
	ObjectInitialization() {}
	ObjectInitialization(int a, ObjectInitializationFunc f, bool c) : AnimSetID(a), Function(f), CreateObjectFromEventMap(c) {}

	void AddObject(Level&, int, int) const;
};