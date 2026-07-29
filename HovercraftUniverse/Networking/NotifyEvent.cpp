#include "NotifyEvent.h"

namespace HovUni {

NotifyEvent::NotifyEvent(const std::string& notif) :
	ChatEvent(notifyLine, notif) {

}

NotifyEvent::NotifyEvent() :
	ChatEvent(notifyLine, "") {

}

NotifyEvent::~NotifyEvent() {

}

NotifyEvent* NotifyEvent::parse(ZCom_BitStream* stream) {
	NotifyEvent* e = new NotifyEvent();
	e->deserialize(stream);
	return e;
}

void NotifyEvent::write(ZCom_BitStream* stream) const {
	stream->addString(mLine.c_str());
}

void NotifyEvent::read(ZCom_BitStream* stream) {
	mLine = std::string(stream->getString());
}

}
