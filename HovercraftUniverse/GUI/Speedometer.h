#ifndef _SPEEDOMETER_H
#define _SPEEDOMETER_H

#include "BasicOverlay.h"

namespace HovUni {
	/**
	 * An overlay containing a speedometer representation.
	 *
	 * @author Nick De Frangh
	 */
	class Speedometer : public BasicOverlay {
		public:
			/**
			 * Speedometer constructor, see BasicOverlay for parameter description
			 */
			Speedometer(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder = 0);
			
			/**
			 * Set the speedometer to a certain speed [0 - 100]
			 *
			 * @param speed The value to set the speedometer to [0 - 100]
			 */
			void setSpeed(Ogre::Real speed);

			/**
			 * Set the boost meter to a certain value [0 - 100]
			 *
			 * @param boost The value to set the boost meter to [0 - 100]
			 */
			void Speedometer::setBoost(Ogre::Real boost);

			/**
			 * Gently move to a new speed setting with a given step size
			 *
			 * @param speed The target speed to move to
			 * @param step The steps to take
			 */
			void moveToSpeed(Ogre::Real speed, Ogre::Real step);
	};
}

#endif //_SPEEDOMETER_H