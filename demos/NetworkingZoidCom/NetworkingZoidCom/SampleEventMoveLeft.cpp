#include "StdAfx.h"
#include "SampleEventMoveLeft.h"

namespace HovUni {

SampleEventMoveLeft::SampleEventMoveLeft(float distance) : NetworkEvent(MoveLeft), mDistance(distance) {

}

SampleEventMoveLeft::SampleEventMoveLeft() : NetworkEvent(MoveForward), mDistance(0.0) {

}

SampleEventMoveLeft::~SampleEventMoveLeft() {

}

float SampleEventMoveLeft::getDistance() const {
	return mDistance;
}

void SampleEventMoveLeft::write(ZCom_BitStream* stream) const {
	stream->addFloat(mDistance, 23);
}

void SampleEventMoveLeft::read(ZCom_BitStream* stream) {
	mDistance = stream->getFloat(23);
}

SampleEventMoveLeft* SampleEventMoveLeft::parse(ZCom_BitStream* stream) {
	SampleEventMoveLeft* e = new SampleEventMoveLeft;
	e->deserialize(stream);
	return e;
}

}
