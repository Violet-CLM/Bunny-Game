#pragma once
#include <memory>
#include <functional>
#include "Resources.h"
#include "Layer.h"
#include "Event.h"

class ObjectStartPos {
	friend class GameObject;
private:
	float OriginX, OriginY;
	const AnimSet* Set;
	const Level& HostLevel;
	Event& HostEvent;
public:
	ObjectStartPos(const Level& L, Event& E, float X, float Y, const AnimSet* S) : HostLevel(L), HostEvent(E), OriginX(X), OriginY(Y), Set(S) {}
};

class GameState;
class GameObject {
	friend class ObjectInitialization;

public:
	float PositionX, PositionY;

	bool CollidesWith(const GameObject&) const;
	AnimFrame& GetFrame() const;

	GameObject( ObjectStartPos& objStart) : HostLevel(objStart.HostLevel), HostEvent(objStart.HostEvent), Active(true), ObjectType(0) {
		PositionX = OriginX = objStart.OriginX;
		PositionY = OriginY = objStart.OriginY;
		Set = objStart.Set;
	}

	float OriginX, OriginY;
private:
	const AnimSet* Set;
	const Level& HostLevel;
public:
	unsigned int AnimID, FrameID;
	bool Active;
	unsigned int ObjectType; //no inherent use

	virtual void Behave(GameState&) = 0;
	virtual void Draw(Layer*) const = 0;
	virtual void HitBy(GameObject&) {}
protected:
	Event& HostEvent;
	unsigned int RadiusX, RadiusY;
	bool RoundedCorners;

	unsigned int GetFrameCount() const;
	void DetermineFrame(unsigned int);
	void Delete();
	void Deactivate();
};

typedef GameObject* (*ObjectInitializationFunc)(ObjectStartPos&);
typedef bool ObjectCollisionTestFunction(const GameObject&, const GameObject&);

class ObjectInitialization {
public:
	int AnimSetID;
	ObjectInitializationFunc Function;
	bool CreateObjectFromEventMap;
	ObjectInitialization() {}
	ObjectInitialization(int a, ObjectInitializationFunc f, bool c) : AnimSetID(a), Function(f), CreateObjectFromEventMap(c) {}

	void AddObject(Level&, Event& ev, int, int) const;
};