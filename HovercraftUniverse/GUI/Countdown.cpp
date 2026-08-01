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
			// Ogre 14 / modern-MSVC fix: Hikari::FlashValue has both an
			// `int` and an `Ogre::Real` (float) converting constructor, so
			// passing a bare `long` here is ambiguous under standard
			// overload resolution (both are equally-ranked standard
			// conversions) -- this apparently slipped through on the
			// original VC9 toolchain but is a hard error on v143. Disambiguate
			// explicitly; mTime is a countdown in milliseconds, always well
			// within `int` range.
			this->callFunction("start", Hikari::Args((int)mTime));
		} catch (OverlayNotActivatedException) {
			//Ignore
		}
	}

	void Countdown::resync(long milliseconds) {
		if (mStarted) {
			try {
				// See the comment in Countdown::start above.
				this->callFunction("resync", Hikari::Args((int)milliseconds));
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