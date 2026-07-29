#ifndef _LOADINGOVERLAY_H
#define _LOADINGOVERLAY_H

#include "BasicOverlay.h"

namespace HovUni {
	/**
	 * This overlay represents a Loading screen
	 *
	 * @author Nick De Frangh
	 */
	class LoadingOverlay : public BasicOverlay {
		private:
			/** The current loaded value */
			Ogre::Real mLoadedValue;

			/** The current loaded message */
			Ogre::String mLoadedMessage;

		public:
			/**
			 * Basic constructor, see BasicOverlay for info about the parameters
			 */
			LoadingOverlay(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder = 0);

			/**
			 * Set the loaded percentage [0 - 100]
			 *
			 * @param loaded the percentage to show [0 - 100]
			 * @param message A Message to be displayed
			 */
			void setLoaded(Ogre::Real loaded, const Ogre::String& message);

			/**
			 * Function that will be called when the overlay is activated
			 */
			virtual void customActionAfterActivate();
	};
}

#endif //_LOADINGOVERLAY_H