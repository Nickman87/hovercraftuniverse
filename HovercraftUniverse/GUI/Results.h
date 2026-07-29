#ifndef _RESULTS_H
#define _RESULTS_H

#include "BasicOverlay.h"

namespace HovUni {
	/**
	 * An overlay containing a results representation.
	 *
	 * @author Tobias Van Bladel & Nick De Frangh
	 */
	class Results : public BasicOverlay {

		public:
			/**
			 * Result constructor, see BasicOverlay for parameter description
			 */
			Results(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder = 0);

			/**
			 * add a player
			 *
			 * @return true if the update succeeded, false otherwise
			 */
			bool addPlayer(int position, const Ogre::String& name, long time);
	};
}


#endif //_COUNTDOWN_H