#ifndef _CONNECTWINDOW_H
#define _CONNECTWINDOW_H

#include "BasicOverlay.h"

namespace HovUni {
	/**
	 * An overlay containing a connect window
	 *
	 * @author Nick De Frangh
	 */
	class ConnectWindow : public BasicOverlay {
		public:
			ConnectWindow(const Hikari::FlashDelegate& okCallback, const Hikari::FlashDelegate& cancelCallback, const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder = 0);

			void customActionAfterActivate();
	};
}

#endif //_CONNECTWINDOW_H