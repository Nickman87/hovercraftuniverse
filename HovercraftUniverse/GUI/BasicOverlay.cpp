#include "BasicOverlay.h"

#include "GUIManager.h"
#include "Exception.h"

namespace HovUni {
	BasicOverlay::BasicOverlay(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position &position, Ogre::ushort zOrder) 
		: mName(name), mFileName(fileName), mWidth(width), mHeight(height), mPosition(position), mZOrder(zOrder), mFlashControl(0), mIgnoreInputs(false), mExactFit(false), mNoBorders(false) {
		//Set default values for parameters
		mParameters_B[TRANSPARANCY] = true;
		mParameters_B[ALPHAHACK] = false;
	}

	BasicOverlay::~BasicOverlay() {
		
	}

	bool BasicOverlay::keyPressed(const OIS::KeyEvent &evt) {
		if (mFlashControl) {
			//mFlashControl->handleInputs(
		}
		return true;
	}

	bool BasicOverlay::keyReleased(const OIS::KeyEvent &evt) {
		return true;
	}

	void BasicOverlay::registerForKeys() {
		GUIManager::getSingletonPtr()->addKeyListener(this);
	}

	void BasicOverlay::unregisterForKeys() {
		GUIManager::getSingletonPtr()->removeKeyListener(this);
	}

	Hikari::FlashControl* BasicOverlay::getFlashControl() {
		return mFlashControl;
	}

	void BasicOverlay::activate() {
		customActionBeforeActivate();

		mFlashControl = GUIManager::getSingletonPtr()->createOverlay(mName, mFileName, mWidth, mHeight, mPosition, mZOrder);
		mFlashControl->setDraggable(false);
		if (mExactFit) {
			mFlashControl->setScaleMode(Hikari::SM_EXACTFIT);
		} else if (mNoBorders) {
			mFlashControl->setScaleMode(Hikari::SM_NOBORDER);
		} else {
			mFlashControl->setScaleMode(Hikari::SM_SHOWALL);
		}

		if (mParameters_B[TRANSPARANCY]) {
			mFlashControl->setTransparent(mParameters_B[TRANSPARANCY], mParameters_B[ALPHAHACK]);
		} else {
			mFlashControl->setTransparent(false, false);
		}

		//Set the input ignore value
		mFlashControl->handleInputs(!mIgnoreInputs);

		//Register callbacks
		for (unsigned int i = 0; i < mCallbacks.size(); ++i) {
			std::pair<Ogre::DisplayString, Hikari::FlashDelegate> callback = mCallbacks[i];

			mFlashControl->bind(callback.first, callback.second);
		}

		customActionAfterActivate();
	}

	void BasicOverlay::disable() {
		customActionBeforeDeactivate();

		GUIManager::getSingletonPtr()->removeOverlay(mName);
		//Remove reference to the pointer since the object will be deleted in the next update
		mFlashControl = 0;

		customActionAfterDeactivate();
	}

	Hikari::FlashValue BasicOverlay::callFunction(const Ogre::DisplayString& name, const Hikari::Arguments& args) {
		if (mFlashControl) {
			return mFlashControl->callFunction(name, args);
		} else {
			//Exception
			return "false";
			//THROW(OverlayNotActivatedException, "You need to activate the overlay before you can call functions on it");
		}
	}

	void BasicOverlay::setBParameter(enum OverlayParams_B paramIndex, bool value) {
		mParameters_B[paramIndex] = value;
	}

	bool BasicOverlay::getBParameter(enum OverlayParams_B paramIndex) {
		return mParameters_B[paramIndex];
	}

	void BasicOverlay::bind(const Ogre::DisplayString& funcName, const Hikari::FlashDelegate& callback) {
		//Store the bind
		std::pair<Ogre::DisplayString, Hikari::FlashDelegate> callStore(funcName, callback);
		mCallbacks.push_back(callStore);
	}

	void BasicOverlay::ignoreInputs(bool val) {
		mIgnoreInputs = val;

		if (mFlashControl) {
			mFlashControl->handleInputs(!mIgnoreInputs);
		}
	}

	void BasicOverlay::focus() {
		if (mFlashControl) {
			mFlashControl->focus();
		}
	}

	void BasicOverlay::setExactFit(bool val) {
		mExactFit = val;
	}

	void BasicOverlay::setNoBorders(bool val) {
		mNoBorders = val;
	}
}