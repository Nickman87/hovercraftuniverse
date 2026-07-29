#include "CheckpointEvent.h"
#include "GameEventType.h"

namespace HovUni {

CheckpointEvent::CheckpointEvent(unsigned int user, unsigned int checkpoint, long time) :
	GameEvent(checkPoint), mUser(user), mCheckpoint(checkpoint), mTimestamp(time) {

}

CheckpointEvent::CheckpointEvent() :
	GameEvent(checkPoint), mUser(0), mCheckpoint(-1), mTimestamp(-1) {

}

CheckpointEvent::~CheckpointEvent() {

}

void CheckpointEvent::write(ZCom_BitStream* stream) const {
	stream->addInt(mUser, 32);
	stream->addInt(mCheckpoint, 32);
	// TODO Replicate the whole long
	stream->addInt(mTimestamp, 32);
}

void CheckpointEvent::read(ZCom_BitStream* stream) {
	mUser = stream->getInt(32);
	mCheckpoint = stream->getInt(32);
	mTimestamp = stream->getInt(32);
}

CheckpointEvent * CheckpointEvent::parse(ZCom_BitStream* stream) {
	CheckpointEvent * result = new CheckpointEvent();
	result->deserialize(stream);
	return result;
}

}
