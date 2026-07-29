#include "NMessageBox.h"
#include "GUIManager.h"

namespace HovUni {
	NMessageBox::NMessageBox(const Ogre::String& message, const Ogre::String& name, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder)
		: BasicOverlay(name, "messagebox.swf", width, height, position, zOrder), mMessage(message) {
		this->bind("onOk", Hikari::FlashDelegate(this, &NMessageBox::onOk));
	}

	void NMessageBox::customActionAfterActivate() {
		//Set the text
		callFunction("setText", Hikari::Args(mMessage));
	}

	Hikari::FlashValue NMessageBox::onOk(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		GUIManager::getSingletonPtr()->disableOverlay(this);
		delete this;

		return "success";
	}
}