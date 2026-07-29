#include "ChatEvent.h"

namespace HovUni {

ChatEvent::ChatEvent(ChatEventType type, const std::string& line) :
	NetworkEvent<ChatEventType, ChatEventTypeSize,1> (type), mLine(line) {

}

ChatEvent::~ChatEvent() {

}

std::string ChatEvent::getLine() const {
	return mLine;
}

}
