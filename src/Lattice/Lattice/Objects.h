#pragma once
#include <memory>
#include <functional>
#include <vector>
#include <forward_list>
#include "Resources.h"
#include "Layer.h"
#include "Event.h"

class Level;
class ObjectStartPos {
	friend class GameObject;
private:
	float OriginX, OriginY;
	const AnimSet* Set;
	Level& HostLevel;
	Event& HostEvent;
public:
	ObjectStartPos(Level& L, Event& E, float X, float Y, const AnimSet* S) : HostLevel(L), HostEvent(E), OriginX(X), OriginY(Y), Set(S) {}
};

struct ObjectCollisionShape {
	float OffsetX, OffsetY;
	union {
		unsigned int Width;
		unsigned int Radius;
	};
	union {
		unsigned int Height;
		bool IsRectangle;
	};
	ObjectCollisionShape(unsigned int R, float X, float Y) : Radius(R), Height(0), OffsetX(X), OffsetY(Y) {} //circle
	ObjectCollisionShape(unsigned int R) : ObjectCollisionShape(R, 0, 0) {} //local circle
	ObjectCollisionShape(unsigned int W, unsigned int H, float X, float Y) : Width(W), Height(H), OffsetX(X), OffsetY(Y) {} //rectangle
	ObjectCollisionShape(unsigned int W, unsigned int H) : ObjectCollisionShape(W, H, W/-2.f, H/-2.f) {} //local rectangle

	bool CollidesWith(float, float, const ObjectCollisionShape&, float, float) const;
};

class GameState;
class GameObject {
	friend class ObjectInitialization;

public:
	float PositionX, PositionY;

	bool CollidesWith(const GameObject&) const;
	AnimFrame& GetFrame() const;

	GameObject(ObjectStartPos& objStart);

	float OriginX, OriginY;
private:
	Level& HostLevel;
public:
	const AnimSet* Set;
	unsigned int AnimID, FrameID;
	bool Active;
	unsigned int ObjectType; //no inherent use

	virtual void Behave(GameState&) = 0;
	virtual void Draw(Layer*) const = 0;
	virtual void HitBy(GameObject&) {}

	GameObject& AddObject(EventID, float, float, bool=false);
protected:
	Event& HostEvent;
	std::forward_list<std::unique_ptr<GameObject>>& HostLevelObjectList;
	std::vector<ObjectCollisionShape> CollisionShapes;

	GameObject* Parent;
	std::list<GameObject*> Children;
	virtual void LostParent();
	virtual void LostChild(GameObject&);

	unsigned int GetFrameCount() const;
	void DetermineFrame(unsigned int);
	void Delete();
	void Deactivate();

};

typedef GameObject* (*ObjectInitializationFunc)(ObjectStartPos&);
typedef bool ObjectCollisionTestFunction(const GameObject&, const GameObject&);
typedef void HUDUpdateFunction(VertexCollectionQueue&, unsigned int);

class ObjectInitialization {
public:
	int AnimSetID;
	ObjectInitializationFunc Function;
	bool CreateObjectFromEventMap;
	ObjectInitialization() {}
	ObjectInitialization(int a, ObjectInitializationFunc f, bool c) : AnimSetID(a), Function(f), CreateObjectFromEventMap(c) {}

	GameObject& AddObject(Level&, Event& ev, float, float) const;
};