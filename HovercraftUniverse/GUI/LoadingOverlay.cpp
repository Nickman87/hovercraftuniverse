#include "LoadingOverlay.h"
#include "Exception.h"

namespace HovUni {

	LoadingOverlay::LoadingOverlay(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder) 
		: BasicOverlay(name, fileName, width, height, position, zOrder), mLoadedValue(0.0f), mLoadedMessage("") {
			this->setBParameter(BasicOverlay::TRANSPARANCY, false);
	}
	
	void LoadingOverlay::setLoaded(Ogre::Real loaded, const Ogre::String& message) {
		mLoadedValue = loaded;
		mLoadedMessage = message;

		try {
			this->callFunction("setLoaded", Hikari::Args(loaded)(message));
		} catch (OverlayNotActivatedException) {
			//Ignore...
		}
	}

	void LoadingOverlay::customActionAfterActivate() {
		//Make sure we display the correct percentage
		setLoaded(mLoadedValue, mLoadedMessage);
	}
}