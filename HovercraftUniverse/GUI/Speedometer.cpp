#include "Speedometer.h"

namespace HovUni {
	Speedometer::Speedometer(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder) 
		: BasicOverlay(name, fileName, width, height, position, zOrder) {
	}

	void Speedometer::setSpeed(Ogre::Real speed) {
		this->callFunction("setSpeed", Hikari::Args(speed));
	}

	void Speedometer::setBoost(Ogre::Real boost) {
		this->callFunction("setBoost", Hikari::Args(boost));
	}

	void Speedometer::moveToSpeed(Ogre::Real speed, Ogre::Real step) {
		this->callFunction("moveToSpeed", Hikari::Args(speed)(step));
	}
}