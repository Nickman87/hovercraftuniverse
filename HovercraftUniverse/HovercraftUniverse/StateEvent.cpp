#include "StateEvent.h"
#include "GameEventType.h"

namespace HovUni {

StateEvent::StateEvent(unsigned int state) :
	GameEvent(stateChange), mState(state) {
}

StateEvent::StateEvent() :
	GameEvent(stateChange) {

}

StateEvent::~StateEvent() {
}

void StateEvent::write(ZCom_BitStream* stream) const {
	stream->addInt(mState, 4);
}

void StateEvent::read(ZCom_BitStream* stream) {
	mState = stream->getInt(4);
}

StateEvent * StateEvent::parse(ZCom_BitStream* stream) {
	StateEvent * result = new StateEvent();
	result->deserialize(stream);
	return result;
}

}
