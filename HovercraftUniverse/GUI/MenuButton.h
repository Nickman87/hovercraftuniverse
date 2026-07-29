#ifndef _MENUBUTTON_H
#define _MENUBUTTON_H

#include "BasicOverlay.h"

namespace HovUni {
	/**
	 * An overlay containing a menu button
	 *
	 * @author Nick De Frangh
	 */
	class MenuButton : public BasicOverlay {
		private:
			/** The text to be displayed on the button */
			Ogre::String mText; 

		public:
			/**
			 * MenuButton constructor, see BasicOverlay for parameter description
			 */
			MenuButton(const Ogre::String& text, const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder = 0);

			/**
			 * Call this function to make sure the Pressed glow keeps visible
			 * even after the object went out of scope
			 */
			void keepPressed();

			/**
			 * Call this function to cancel the press effect. This should be called
			 * if you want to be sure the press effect will dissapear when it has to
			 * even when the object is out of focus.
			 */
			void cancelPress();

		protected:
			/** Enter the given text into the button */
			void setText();

			virtual void customActionAfterActivate();
	};
}

#endif //_MENUBUTTON_H