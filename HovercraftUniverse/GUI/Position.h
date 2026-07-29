#ifndef _POSITION_H
#define _POSITION_H

#include "BasicOverlay.h"

namespace HovUni {
	/**
	 * An overlay containing a position representation.
	 *
	 * @author Nick De Frangh
	 */
	class Position : public BasicOverlay {
		public:
			/**
			 * Position constructor, see BasicOverlay for parameter description
			 */
			Position(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder = 0);

			void setNumberOfPlayers(int players);

			void setCurrentPosition(int position);
	};
}

#endif //_POSITION_H