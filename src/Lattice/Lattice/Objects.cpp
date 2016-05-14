#include "Objects.h"
#include "Level.h"

bool GameObject::IsActive() const
{
	return Behavior != nullptr;
}

void GameObject::Behave(Level& level)
{
	if (IsActive())
		Behavior->Behave(level);
}
void GameObject::Draw(Layer* layers) const
{
	if (IsActive())
		Behavior->Draw(layers);
}

bool GameObject::CollidesWith(const GameObject& other) const
{
	if (!RoundedCorners && !other.RoundedCorners) { //both rectangular
		if ((PositionX - RadiusX) > (other.PositionX + RadiusX))
			return false;
		if ((PositionX + RadiusX) < (other.PositionX - RadiusX))
			return false;
		if ((PositionY - RadiusY) > (other.PositionY + RadiusY))
			return false;
		if ((PositionY + RadiusY) < (other.PositionY - RadiusY))
			return false;
		return true;
	}
	return false;
}

AnimFrame & GameObject::GetFrame() const
{
	return (*Set->Animations[AnimID].AnimFrames)[FrameID];
}

void ObjectInitialization::AddObject(Level& level, int x, int y) const
{
	level.Objects.push_back(std::unique_ptr<GameObject>(Function(new GameObject(x, y, AnimSets[AnimSetID], 0, 0))));
}

void GameObjectBehavior::DetermineFrame(int frameID)
{
	const int frameCount = BasicProperties->Set->Animations[BasicProperties->AnimID].AnimFrames->size();
	BasicProperties->FrameID = (!!frameCount) ? (frameID % frameCount) : 0;
}
void GameObjectBehavior::DetermineFrame(int animID, int frameID)
{
	BasicProperties->AnimID = animID;
	DetermineFrame(frameID);
}
