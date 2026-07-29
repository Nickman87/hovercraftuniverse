#ifndef _TIMEBEHIND_H
#define _TIMEBEHIND_H

#include "BasicOverlay.h"

namespace HovUni {
	/**
	 * An overlay containing a time behind representation.
	 *
	 * @author Nick De Frangh
	 */
	class TimeBehind : public BasicOverlay {
		public:
			/**
			 * TimeBehind constructor, see BasicOverlay for parameter description
			 */
			TimeBehind(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder = 0);

	};
}

#endif //_TIMEBEHIND_H