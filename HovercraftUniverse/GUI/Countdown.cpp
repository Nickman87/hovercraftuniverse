#include "Countdown.h"
#include "Exception.h"

namespace HovUni {
	Countdown::Countdown(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder) 
			: BasicOverlay(name, fileName, width, height, position, zOrder), mStarted(false) {
		this->setBParameter(BasicOverlay::ALPHAHACK, true);
	}

	void Countdown::start(long milliseconds) {
		mStarted = true;
		mTime = milliseconds;

		try {
			this->callFunction("start", Hikari::Args(mTime));
		} catch (OverlayNotActivatedException) {
			//Ignore
		}
	}

	void Countdown::resync(long milliseconds) {
		if (mStarted) {
			try {
				this->callFunction("resync", Hikari::Args(milliseconds));
			} catch (OverlayNotActivatedException) {
				mTime = milliseconds;
			}
		}
	}

	void Countdown::customActionAfterActivate() {
		if (mStarted) {
			start(mTime);
		}
	}
}