#ifndef _DIRECTION_H
#define _DIRECTION_H

#include "BasicOverlay.h"

namespace HovUni {
	/**
	 * An overlay containing a direction representation.
	 *
	 * @author Nick De Frangh
	 */
	class Direction : public BasicOverlay {
		public:
			/**
			 * Direction constructor, see BasicOverlay for parameter description
			 */
			Direction(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder = 0);

			/**
			 * Set the angle of the direction arrow
			 */
			void setAngle(Ogre::Real angle);
	};
}

#endif //_DIRECTION_H