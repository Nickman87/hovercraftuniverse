#include "ChatEventParser.h"
#include "TextEvent.h"
#include "NotifyEvent.h"

namespace HovUni {

ChatEventParser::~ChatEventParser() {

}

ChatEvent* ChatEventParser::parse(ZCom_BitStream* stream) {
	if ( !ChatEvent::checkEventClass(stream) ){
		return 0;
	}

	ChatEventType type = ChatEvent::readType(stream);

	switch (type) {
	case textLine:
		return TextEvent::parse(stream);
		break;
	case notifyLine:
		return NotifyEvent::parse(stream);
		break;
	default:
		return 0;
		break;
	}
}

}
