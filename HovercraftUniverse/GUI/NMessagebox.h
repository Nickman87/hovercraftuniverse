#ifndef _MESSAGEBOX_H
#define _MESSAGEBOX_H

#include "BasicOverlay.h"

namespace HovUni {
	/**
	 * A simple messageBox overlay.
	 *
	 * @author Nick De Frangh
	 */
	class NMessageBox : public BasicOverlay {
		private:
			/** The message to be displayed */
			Ogre::String mMessage;

		public:
			/**
			 * Create a messagebox.
			 * 
			 * @param message The message to display
			 */
			NMessageBox(const Ogre::String& message, const Ogre::String& name, int width = 520, int height = 150, const Hikari::Position& position = Hikari::Center, Ogre::ushort zOrder = 0);

			/**
			 * Function that gets called after activation. This will set the text
			 * of the messagebox.
			 */
			virtual void customActionAfterActivate();

			/**
			 * Function that will be executed when a user presses the Ok button.
			 * This will automaticly disable the overlay.
			 *
			 * @param caller The calling flash object. Don't use this unless you know how!
			 * @param args The arguments passed from the overlay (empty)
			 */
			Hikari::FlashValue onOk(Hikari::FlashControl* caller, const Hikari::Arguments& args);
	};
}

#endif //_MESSAGEBOX_H