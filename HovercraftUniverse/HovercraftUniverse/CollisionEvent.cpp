#include "CollisionEvent.h"
#include "VisualEventType.h"

namespace HovUni {

CollisionEvent::CollisionEvent(const Ogre::Vector3& position, const Ogre::Vector3& normal):
	VisualEvent(onCollision), mPosition(position), mNormal(normal)
{
}

CollisionEvent::CollisionEvent(const CollisionEvent& c): 
	VisualEvent(onCollision), mPosition(c.getPosition()), mNormal(c.getNormal()) {}

CollisionEvent::CollisionEvent():
	VisualEvent(onCollision){
}

CollisionEvent::~CollisionEvent(void)
{
}

void CollisionEvent::write(ZCom_BitStream* stream) const {
	//write position
	stream->addFloat(mPosition[0],4);
	stream->addFloat(mPosition[1],4);
	stream->addFloat(mPosition[2],4);

	//write normal
	stream->addFloat(mNormal[0],4);
	stream->addFloat(mNormal[1],4);
	stream->addFloat(mNormal[2],4);
}

void CollisionEvent::read(ZCom_BitStream* stream) {
	mPosition[0] = stream->getFloat(4);
	mPosition[1] = stream->getFloat(4);
	mPosition[2] = stream->getFloat(4);

	mNormal[0] = stream->getFloat(4);
	mNormal[1] = stream->getFloat(4);
	mNormal[2] = stream->getFloat(4);
}

CollisionEvent * CollisionEvent::parse(ZCom_BitStream* stream) {
	CollisionEvent * result = new CollisionEvent();
	result->deserialize(stream);
	return result;
}

}
