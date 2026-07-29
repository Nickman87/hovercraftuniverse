#include "StdAfx.h"
#include "SampleEventMoveRight.h"

namespace HovUni {

SampleEventMoveRight::SampleEventMoveRight(float distance) : NetworkEvent(MoveRight), mDistance(distance) {

}

SampleEventMoveRight::SampleEventMoveRight() : NetworkEvent(MoveForward), mDistance(0.0) {

}

SampleEventMoveRight::~SampleEventMoveRight() {

}

float SampleEventMoveRight::getDistance() const {
	return mDistance;
}

void SampleEventMoveRight::write(ZCom_BitStream* stream) const {
	stream->addFloat(mDistance, 23);
}

void SampleEventMoveRight::read(ZCom_BitStream* stream) {
	mDistance = stream->getFloat(23);
}

SampleEventMoveRight* SampleEventMoveRight::parse(ZCom_BitStream* stream) {
	SampleEventMoveRight* e = new SampleEventMoveRight;
	e->deserialize(stream);
	return e;
}

}
