#include "InitEvent.h"
#include "GameEventType.h"

namespace HovUni {

InitEvent::InitEvent(ZCom_BitStream* stream) :
	GameEvent(init), mStream(stream) {
}

InitEvent::InitEvent() :
	GameEvent(init) {

}

InitEvent::~InitEvent() {
}

void InitEvent::write(ZCom_BitStream* stream) const {
	stream->addInt(mStream->getBitCount(), 32);
	stream->addBitStream(mStream, true);
}

void InitEvent::read(ZCom_BitStream* stream) {
	zU32 bits = stream->getInt(32);
	mStream = stream->getBitStream(bits, true);
}

InitEvent * InitEvent::parse(ZCom_BitStream* stream) {
	InitEvent * result = new InitEvent();
	result->deserialize(stream);
	return result;
}

}
