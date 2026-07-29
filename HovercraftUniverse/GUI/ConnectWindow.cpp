#include "ConnectWindow.h"

namespace HovUni {
	ConnectWindow::ConnectWindow(const Hikari::FlashDelegate& okCallback, const Hikari::FlashDelegate& cancelCallback, const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder) 
			: BasicOverlay(name, fileName, width, height, position, zOrder) {
		setBParameter(BasicOverlay::ALPHAHACK, true);
		//Bind callback functions
		bind("onOk", okCallback);
		bind("onCancel", cancelCallback);
	}

	void ConnectWindow::customActionAfterActivate() {
		this->focus();
	}
}