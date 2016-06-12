#include "Objects.h"
#include "Level.h"

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

void ObjectInitialization::AddObject(Level& level, Event& ev, int x, int y) const
{
	level.Objects.emplace_front(Function(ObjectStartPos(level, ev, float(x), float(y), AnimationSets[AnimSetID])));
}

unsigned int GameObject::GetFrameCount() const {
	return Set->Animations[AnimID].AnimFrames->size();
}
void GameObject::DetermineFrame(unsigned int frameID)
{
	const unsigned int frameCount = GetFrameCount();
	FrameID = (!!frameCount) ? (frameID % frameCount) : 0;
}

void GameObject::Delete()
{
	Active = false;
}

void GameObject::Deactivate()
{
	Delete();
	HostEvent.Active = false;
}
