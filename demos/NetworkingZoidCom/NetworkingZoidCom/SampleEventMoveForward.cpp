#include "StdAfx.h"
#include "SampleEventMoveForward.h"

namespace HovUni {

SampleEventMoveForward::SampleEventMoveForward(float distance) : NetworkEvent(MoveForward), mDistance(distance) {

}

SampleEventMoveForward::SampleEventMoveForward() : NetworkEvent(MoveForward), mDistance(0.0) {

}

SampleEventMoveForward::~SampleEventMoveForward() {

}

float SampleEventMoveForward::getDistance() const {
	return mDistance;
}

void SampleEventMoveForward::write(ZCom_BitStream* stream) const {
	stream->addFloat(mDistance, 23);
}

void SampleEventMoveForward::read(ZCom_BitStream* stream) {
	mDistance = stream->getFloat(23);
}

SampleEventMoveForward* SampleEventMoveForward::parse(ZCom_BitStream* stream) {
	SampleEventMoveForward* e = new SampleEventMoveForward;
	e->deserialize(stream);
	return e;
}

}
