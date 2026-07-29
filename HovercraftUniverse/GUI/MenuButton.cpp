#include "MenuButton.h"

namespace HovUni {
	MenuButton::MenuButton(const Ogre::String& text, const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder)
			: BasicOverlay(name, fileName, width, height, position, zOrder), mText(text) {
		setBParameter(BasicOverlay::ALPHAHACK, true);
	}

	void MenuButton::keepPressed() {
		this->callFunction("stickyPress", Hikari::Args());
	}

	void MenuButton::setText() {
		this->callFunction("setText", Hikari::Args(mText));
	}

	void MenuButton::cancelPress() {
		this->callFunction("cancelPress", Hikari::Args());
	}

	void MenuButton::customActionAfterActivate() {
		//Set the text of the button
		setText();
	}
}