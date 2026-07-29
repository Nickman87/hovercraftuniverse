#include "Chat.h"

namespace HovUni {
	Chat::Chat(const Hikari::FlashDelegate& chatInput, const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder)
			: BasicOverlay(name, fileName, width, height, position, zOrder) {
		this->bind("chatInput", chatInput);
		setBParameter(BasicOverlay::ALPHAHACK, true);
	}

	void Chat::addText(const Ogre::String& username, const Ogre::String& message) {
		this->callFunction("addText", Hikari::Args(username)(message));
	}

	void Chat::addAction(const Ogre::String& message) {
		this->callFunction("addAction", Hikari::Args(message));
	}

	void Chat::focusChat() {
		this->callFunction("giveFocus", Hikari::Args());
		focus();
	}

	void Chat::defocusChat() {
		this->callFunction("removeFocus", Hikari::Args());
	}
}