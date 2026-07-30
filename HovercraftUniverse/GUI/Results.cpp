#include "Results.h"
#include "Exception.h"

namespace HovUni {
	Results::Results(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder) 
			: BasicOverlay(name, fileName, width, height, position, zOrder) {
		this->setBParameter(BasicOverlay::ALPHAHACK, true);
	}

	bool Results::addPlayer(int position, const Ogre::String& name, long time) {
		try{
			// See Countdown.cpp's Countdown::start for why the `long` needs
			// an explicit int cast here (ambiguous FlashValue(int) vs.
			// FlashValue(Ogre::Real) overload resolution under v143).
			this->callFunction("addPosition", Hikari::Args(position)(name)((int)time));
		} catch (OverlayNotActivatedException) {
			//Ignore
			return false;
		}
		return true;
	}
}