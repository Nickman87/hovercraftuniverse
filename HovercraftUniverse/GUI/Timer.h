#ifndef _TIMER_H
#define _TIMER_H

#include "BasicOverlay.h"

namespace HovUni {
	/**
	 * An overlay containing a timer representation.
	 *
	 * @author Nick De Frangh
	 */
	class Timer : public BasicOverlay {
		public:
			/**
			 * Timer constructor, see BasicOverlay for parameter description
			 */
			Timer(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder = 0);

			/**
			 * Start the timer
			 */
			void start();

			/**
			 * Stop the timer
			 */
			void stop();

			/**
			 * Set the timer to a given value, make sure it is stopped, otherwise
			 * it will be overwritten
			 *
			 * @param minutes The minutes value
			 * @param seconds The seconds value
			 * @param hundreds The hundreds value
			 */
			void setTime(int minutes, int seconds, int hundreds);
	};
}

#endif //_TIMEBEHIND_H