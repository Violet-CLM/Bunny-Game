#pragma once
#include <memory>
#include <functional>
#include "Resources.h"
#include "Layer.h"

class GameObjectBehavior {
	friend class GameObject;
public:
	GameObjectBehavior(GameObject* go) : BasicProperties(go) {}

protected:
	GameObject* BasicProperties;
	virtual void Behave(Level&) = 0;
	virtual void Draw(Layer*) const = 0;

	void DetermineFrame(int);
	void DetermineFrame(int, int);
};

class GameObject {
	friend class GameObjectBehavior;
	friend class ObjectInitialization;

public:
	float PositionX, PositionY;

	bool IsActive() const;
	void Behave(Level&);
	void Draw(Layer*) const;
	bool CollidesWith(const GameObject&) const;
	AnimFrame& GetFrame() const;

	GameObject(int x, int y, const AnimSet* a, unsigned int ai, unsigned int fi) : OriginX(float(x)), OriginY(float(y)), Set(a), AnimID(ai), FrameID(fi) {
		PositionX = OriginX;
		PositionY = OriginY;
	}

	std::unique_ptr<GameObjectBehavior> Behavior;
	float OriginX, OriginY;
private:

	unsigned int RadiusX, RadiusY;
	bool RoundedCorners;

	const AnimSet* Set;
public:
	unsigned int AnimID, FrameID;
};

typedef GameObject* (*ObjectInitializationFunc)(GameObject*);

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