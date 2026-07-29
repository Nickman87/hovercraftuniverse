#include "StdAfx.h"
#include "SampleEventMoveBackward.h"

namespace HovUni {

SampleEventMoveBackward::SampleEventMoveBackward(float distance) : NetworkEvent(MoveBackward), mDistance(distance) {

}

SampleEventMoveBackward::SampleEventMoveBackward() : NetworkEvent(MoveForward), mDistance(0.0) {

}

SampleEventMoveBackward::~SampleEventMoveBackward() {

}

float SampleEventMoveBackward::getDistance() const {
	return mDistance;
}

void SampleEventMoveBackward::write(ZCom_BitStream* stream) const {
	stream->addFloat(mDistance, 23);
}

void SampleEventMoveBackward::read(ZCom_BitStream* stream) {
	mDistance = stream->getFloat(23);
}

SampleEventMoveBackward* SampleEventMoveBackward::parse(ZCom_BitStream* stream) {
	SampleEventMoveBackward* e = new SampleEventMoveBackward;
	e->deserialize(stream);
	return e;
}

}
