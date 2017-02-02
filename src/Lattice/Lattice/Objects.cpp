#include "Objects.h"
#include "Level.h"

static inline bool WithinRadius(float DeltaX, float DeltaY, unsigned int Radius) {
	return DeltaX*DeltaX + DeltaY*DeltaY <= Radius*Radius;
}
static inline bool WithinRadius(float PositionX, float OtherPositionX, float PositionY, float OtherPositionY, unsigned int Radius) {
	return WithinRadius(PositionX - OtherPositionX, PositionY - OtherPositionY, Radius);
}
bool ObjectCollisionShape::CollidesWith(float PositionX, float PositionY, const ObjectCollisionShape& Other, float OtherPositionX, float OtherPositionY) const
{
	if (!IsRectangle && Other.IsRectangle) //reuse code below for this being a rectangle and other being a circle
		return Other.CollidesWith(OtherPositionX, OtherPositionY, *this, PositionX, PositionY);

	PositionX += OffsetX;
	PositionY += OffsetY;
	OtherPositionX += Other.OffsetX;
	OtherPositionY += Other.OffsetY;
	if (!IsRectangle) //this is a circle, other is also a circle
			return WithinRadius(PositionX, OtherPositionX, PositionY, OtherPositionY, Radius + Other.Radius);
	const float Bottom = PositionY + Height;
	const float Right = PositionX + Width;
	if (IsRectangle && Other.IsRectangle) {
		if (PositionX > (OtherPositionX + Other.Width))
			return false;
		if (Right < OtherPositionX)
			return false;
		if (PositionY > (OtherPositionY + Other.Height))
			return false;
		if (Bottom < OtherPositionY)
			return false;
		return true;
	} else { //other must be a circle... in this case, Position* is our top left corner, and OtherPosition* is other's center
		if (PositionX >= OtherPositionX) {
			if (PositionX > OtherPositionX + Other.Radius) //right of right
				return false;
			if (PositionY < OtherPositionY) {
				if (Bottom < OtherPositionY - Other.Radius) //above top
					return false;
				if (OtherPositionY < Bottom && OtherPositionX + Other.Radius >= PositionX) //right edge within rectangle
					return true;
				if (WithinRadius(PositionX, OtherPositionX, Bottom, OtherPositionY, Other.Radius)) //bottom left corner within circle
					return true;
			}
			if (WithinRadius(PositionX, OtherPositionX, PositionY, OtherPositionY, Other.Radius)) //top left corner within circle
				return true;
		} else {
			if (Right < OtherPositionX - Other.Radius) //left of left
				return false;
			if (PositionY >= OtherPositionY) { //top left corner is below and to the left of center
				if (PositionY > OtherPositionY + Other.Radius) //below bottom
					return false;
				if (OtherPositionX < Right && OtherPositionY + Other.Radius >= PositionY) //bottom edge within rectangle
					return true;
				if (WithinRadius(PositionX, OtherPositionX, PositionY, OtherPositionY, Other.Radius)) //top left corner within circle
					return true;
				if (WithinRadius(Right, OtherPositionX, PositionY, OtherPositionY, Other.Radius)) //top right corner within circle
					return true;
			} else if (Bottom < OtherPositionY) { //bottom left corner is above and to the left of center
				if (Bottom < OtherPositionY - Other.Radius) //above top
					return false;
				if (OtherPositionX < Right && OtherPositionY - Other.Radius < Bottom) //top edge within rectangle
					return true;
				//if (WithinRadius(PositionX, OtherPositionX, Bottom, OtherPositionY, Other.Radius)) //bottom left corner within circle
				//	return true;
				if (WithinRadius(Right, OtherPositionX, Bottom, OtherPositionY, Other.Radius)) //bottom right corner within circle
					return true;
			} else { //bottom below center, but top above center; left corners to the left of center, but right corners to the right of right edge
				return true;
			}
		}
		return false;
	}
}
bool GameObject::CollidesWith(const GameObject& other) const
{
	for (auto& shape : CollisionShapes)
		for (auto& otherShape : other.CollisionShapes)
			if (shape.CollidesWith(PositionX, PositionY, otherShape, other.PositionX, other.PositionY))
				return true;
	return false;
}

GameObject::GameObject(ObjectStartPos & objStart): HostLevel(objStart.HostLevel),HostLevelObjectList(HostLevel.Objects),HostEvent(objStart.HostEvent),Active(true),ObjectType(0),Parent(nullptr) {
	PositionX = OriginX = objStart.OriginX;
	PositionY = OriginY = objStart.OriginY;
	Set = objStart.Set;
}

AnimFrame & GameObject::GetFrame() const
{
	return (*Set->Animations[AnimID].AnimFrames)[FrameID];
}
GameObject& ObjectInitialization::AddObject(Level& level, Event& ev, float x, float y) const
{
	GameObject* newObject = Function(ObjectStartPos(level, ev, x, y, AnimationSets[AnimSetID]));
	level.Objects.emplace_front(newObject);
	return *newObject;
}


unsigned int GameObject::GetFrameCount() const {
	return (unsigned int)Set->Animations[AnimID].AnimFrames->size();
}
void GameObject::DetermineFrame(unsigned int frameID)
{
	const unsigned int frameCount = GetFrameCount();
	FrameID = (!!frameCount) ? (frameID % frameCount) : 0;
}

void GameObject::Deactivate()
{
	Delete();
	HostEvent.Active = false;
}
void GameObject::Delete()
{
	Active = false;
	if (Parent != nullptr)
		Parent->LostChild(*this);
	for (auto& child : Children)
		child->LostParent();
}
void GameObject::LostParent()
{
	Parent = nullptr;
}
void GameObject::LostChild(GameObject& child)
{
	Children.remove(&child);
}
void GameObject::Orphan() {
	if (Parent != nullptr)
		Parent->LostChild(*this);
	LostParent();
}
void GameObject::Adopt(GameObject& child) {
	child.Parent = this;
	Children.push_back(&child);
}

GameObject& GameObject::AddObject(EventID eventID, float x, float y, bool parent)
{
	GameObject& newObject = HostLevel.ObjectInitializationListPtr->at(eventID).AddObject(HostLevel, HostLevel.GetEvent(int(x / TILEWIDTH), int(y / TILEHEIGHT)), x, y);
	if (parent)
		Adopt(newObject);
	return newObject;
}
