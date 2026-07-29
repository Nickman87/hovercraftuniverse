#include "Timer.h"

namespace HovUni {
	Timer::Timer(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder)
		: BasicOverlay(name, fileName, width, height, position, zOrder) {
	}

	void Timer::start() {
		this->callFunction("start", Hikari::Args());
	}

	void Timer::stop() {
		this->callFunction("stop", Hikari::Args());
	}

	void Timer::setTime(int minutes, int seconds, int hundreds) {
		this->callFunction("setTime", Hikari::Args(minutes)(seconds)(hundreds));
	}
}