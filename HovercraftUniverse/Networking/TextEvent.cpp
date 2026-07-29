#include "TextEvent.h"

namespace HovUni {

TextEvent::TextEvent(const std::string& user, const std::string& line) :
	ChatEvent(textLine, line), mUser(user) {

}

TextEvent::TextEvent() :
	ChatEvent(textLine, ""), mUser("") {

}

TextEvent::~TextEvent() {

}

std::string TextEvent::getUser() const {
	return mUser;
}

TextEvent* TextEvent::parse(ZCom_BitStream* stream) {
	TextEvent* e = new TextEvent();
	e->deserialize(stream);
	return e;
}

void TextEvent::write(ZCom_BitStream* stream) const {
	stream->addString(mUser.c_str());
	stream->addString(mLine.c_str());
}

void TextEvent::read(ZCom_BitStream* stream) {
	mUser = std::string(stream->getString());
	mLine = std::string(stream->getString());
}

}
