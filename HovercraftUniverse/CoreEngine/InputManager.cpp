#include "InputManager.h"
#include <OgreLogManager.h>

namespace HovUni {

// Singleton object
InputManager * InputManager::mInputManager = 0;

InputManager::InputManager(void) : mMouse(0), mKeyboard(0), mInputSystem(0), mKeyManager(new KeyManager()) {
	// Already initialized
}

InputManager::~InputManager(void) {
	if(mInputSystem) {
		// Destroy mouse
		if(mMouse) {
			mInputSystem->destroyInputObject(mMouse);
			mMouse = 0;
		}

		// Destroy keyboard
		if(mKeyboard) {
			mInputSystem->destroyInputObject(mKeyboard);
			mKeyboard = 0;
		}

		// Destroy joysticks
		if(mJoysticks.size() > 0) {
			for (std::vector<OIS::JoyStick *>::const_iterator it = mJoysticks.begin(); it != mJoysticks.end(); ++it) {
				mInputSystem->destroyInputObject(*it);
			}

			mJoysticks.clear();
		}

		// Destroy input system
		mInputSystem->destroyInputSystem(mInputSystem);
		mInputSystem = 0;

		// Clear Listeners
		mKeyListeners.clear();
		mMouseListeners.clear();
		mJoystickListeners.clear();
	}
}

void InputManager::initialise(Ogre::RenderWindow * renderWindow) {
	if(!mInputSystem) {
		// Setup basic variables
		OIS::ParamList paramList;	
		size_t windowHnd = 0;
		std::ostringstream windowHndStr;

		// Get window handle
		renderWindow->getCustomAttribute("WINDOW", &windowHnd);

		// Fill parameter list
		windowHndStr << (unsigned int) windowHnd;
		paramList.insert(std::make_pair(std::string("WINDOW"), windowHndStr.str()));

		// Create inputsystem
		mInputSystem = OIS::InputManager::createInputSystem(paramList);

		// If possible create a buffered keyboard
		if (mInputSystem->getNumberOfDevices(OIS::OISKeyboard) > 0) {
			mKeyboard = static_cast<OIS::Keyboard*>(mInputSystem->createInputObject(OIS::OISKeyboard, true));
			mKeyboard->setEventCallback(this);
		}

		// If possible create a buffered mouse
		if (mInputSystem->getNumberOfDevices(OIS::OISMouse) > 0) {
			mMouse = static_cast<OIS::Mouse*>(mInputSystem->createInputObject(OIS::OISMouse, true));
			mMouse->setEventCallback(this);

			// Get window size
			unsigned int width, height, depth;
			int left, top;
			renderWindow->getMetrics(width, height, depth, left, top);

			// Set mouse region
			this->setWindowExtents(width, height);
		}

		// If possible create all joysticks in buffered mode
		if (mInputSystem->getNumberOfDevices(OIS::OISJoyStick) > 0) {
			mJoysticks.resize(mInputSystem->getNumberOfDevices(OIS::OISJoyStick));

			for (std::vector<OIS::JoyStick *>::iterator it = mJoysticks.begin(); it != mJoysticks.end(); ++it) {
				(*it) = static_cast<OIS::JoyStick*>(mInputSystem->createInputObject(OIS::OISJoyStick, true));
				(*it)->setEventCallback(this);
			}
		}
	}
}

void InputManager::capture(void) {
	// Need to capture / update each device every frame
	if (mMouse) {
		mMouse->capture();
	}

	if (mKeyboard) {
		mKeyboard->capture();
	}

	if (mJoysticks.size() > 0) {
		for (std::vector<OIS::JoyStick *>::const_iterator it = mJoysticks.begin(); it != mJoysticks.end(); ++it) {
			(*it)->capture();
		}
	}
}

void InputManager::addKeyListener(OIS::KeyListener * keyListener, const std::string & instanceName) {
	if (mKeyboard) {
		// Check for duplicate items
		if (mKeyListeners.find(instanceName) == mKeyListeners.end()) {
			mKeyListeners[instanceName] = keyListener;
		} else {
			// Duplicate Item
		}
	}
}

void InputManager::addMouseListener(OIS::MouseListener *mouseListener, const std::string& instanceName) {
	if (mMouse) {
		// Check for duplicate items
		if (mMouseListeners.find(instanceName) == mMouseListeners.end()) {
			mMouseListeners[instanceName] = mouseListener;
		} else {
			// Duplicate Item
		}
	}
}

void InputManager::addJoystickListener(OIS::JoyStickListener *joystickListener, const std::string& instanceName) {
	if (mJoysticks.size() > 0) {
		// Check for duplicate items
		if (mJoystickListeners.find(instanceName) == mJoystickListeners.end()) {
			mJoystickListeners[instanceName] = joystickListener;
		} else {
			// Duplicate Item
		}
	}
}

void InputManager::removeKeyListener(const std::string& instanceName) {
	// Check if item exists
	if (mKeyListeners.find(instanceName) != mKeyListeners.end()) {
		mKeyListeners.erase(mKeyListeners.find(instanceName));
	} else {
		// Doesn't Exist
	}
}

void InputManager::removeMouseListener(const std::string & instanceName) {
	// Check if item exists
	if (mMouseListeners.find(instanceName) != mMouseListeners.end()) {
		mMouseListeners.erase(mMouseListeners.find(instanceName));
	} else {
		// Doesn't Exist
	}
}

void InputManager::removeJoystickListener(const std::string & instanceName) {
	// Check if item exists
	if (mJoystickListeners.find(instanceName) != mJoystickListeners.end()) {
		mJoystickListeners.erase(mJoystickListeners.find(instanceName));
	}
	else {
		// Doesn't Exist
	}
}

void InputManager::removeKeyListener(OIS::KeyListener * keyListener) {
	for (std::map<std::string, OIS::KeyListener *>::const_iterator it = mKeyListeners.begin(); it != mKeyListeners.end(); ++it) {
		if((*it).second == keyListener) {
			mKeyListeners.erase(it);
			break;
		}
	}
}

void InputManager::removeMouseListener(OIS::MouseListener * mouseListener) {
	for (std::map<std::string, OIS::MouseListener *>::const_iterator it = mMouseListeners.begin(); it != mMouseListeners.end(); ++it) {
		if((*it).second == mouseListener) {
			mMouseListeners.erase(it);
			break;
		}
	}
}

void InputManager::removeJoystickListener(OIS::JoyStickListener *joystickListener) {
	for (std::map<std::string, OIS::JoyStickListener *>::const_iterator it = mJoystickListeners.begin(); it != mJoystickListeners.end(); ++it) {
		if((*it).second == joystickListener) {
			mJoystickListeners.erase(it);
			break;
		}
	}
}

void InputManager::removeAllListeners(void) {
	mKeyListeners.clear();
	mMouseListeners.clear();
	mJoystickListeners.clear();
}

void InputManager::removeAllKeyListeners(void) {
	mKeyListeners.clear();
}

void InputManager::removeAllMouseListeners(void) {
	mMouseListeners.clear();
}

void InputManager::removeAllJoystickListeners(void) {
	mJoystickListeners.clear();
}

void InputManager::setWindowExtents(int width, int height) {
	// Set mouse region (if window resizes, we should alter this to reflect as well)
	const OIS::MouseState& mouseState = mMouse->getMouseState();
	mouseState.width  = width;
	mouseState.height = height;
}

OIS::Mouse* InputManager::getMouse(void) {
	return mMouse;
}

OIS::Keyboard* InputManager::getKeyboard(void) {
	return mKeyboard;
}

OIS::JoyStick* InputManager::getJoystick(unsigned int index) {
	// Make sure it's a valid index
	if(index < mJoysticks.size()) {
		return mJoysticks[index];
	}

	return 0;
}

int InputManager::getNumOfJoysticks(void) {
	return (int) mJoysticks.size();
}

bool InputManager::keyPressed(const OIS::KeyEvent & e) {
	for (std::map<std::string, OIS::KeyListener *>::const_iterator it = mKeyListeners.begin(); it != mKeyListeners.end(); ++it) {
		if(!(*it).second->keyPressed(e)) {
			break;
		}
	}

	return true;
}

bool InputManager::keyReleased(const OIS::KeyEvent & e) {
	for (std::map<std::string, OIS::KeyListener *>::const_iterator it = mKeyListeners.begin(); it != mKeyListeners.end(); ++it) {
		if(!(*it).second->keyReleased(e)) {
			break;
		}
	}

	return true;
}

bool InputManager::mouseMoved(const OIS::MouseEvent & e) {
	for (std::map<std::string, OIS::MouseListener *>::const_iterator it = mMouseListeners.begin(); it != mMouseListeners.end(); ++it) {
		if(!(*it).second->mouseMoved(e))
			break;
	}

	return true;
}

bool InputManager::mousePressed(const OIS::MouseEvent & e, OIS::MouseButtonID id) {
	for (std::map<std::string, OIS::MouseListener *>::const_iterator it = mMouseListeners.begin(); it != mMouseListeners.end(); ++it) {
		if (!(*it).second->mousePressed(e, id)) {
			break;
		}
	}

	return true;
}

bool InputManager::mouseReleased(const OIS::MouseEvent & e, OIS::MouseButtonID id) {
	for (std::map<std::string, OIS::MouseListener *>::const_iterator it = mMouseListeners.begin(); it != mMouseListeners.end(); ++it) {
		if (!(*it).second->mouseReleased(e, id)) {
			break;
		}
	}

	return true;
}

bool InputManager::povMoved(const OIS::JoyStickEvent & e, int pov) {
	for (std::map<std::string, OIS::JoyStickListener *>::const_iterator it = mJoystickListeners.begin(); it != mJoystickListeners.end(); ++it) {
		if (!(*it).second->povMoved(e, pov)) {
			break;
		}
	}

	return true;
}

bool InputManager::axisMoved(const OIS::JoyStickEvent & e, int axis) {
	
	for (std::map<std::string, OIS::JoyStickListener *>::const_iterator it = mJoystickListeners.begin(); it != mJoystickListeners.end(); ++it) {
		if (!(*it).second->axisMoved(e, axis)) {
			break;
		}
	}

	return true;
}

bool InputManager::sliderMoved(const OIS::JoyStickEvent & e, int sliderID) {
	for (std::map<std::string, OIS::JoyStickListener *>::const_iterator it = mJoystickListeners.begin(); it != mJoystickListeners.end(); ++it) {
		if (!(*it).second->sliderMoved(e, sliderID)) {
			break;
		}
	}

	return true;
}

bool InputManager::buttonPressed(const OIS::JoyStickEvent & e, int button) {
	for (std::map<std::string, OIS::JoyStickListener *>::const_iterator it = mJoystickListeners.begin(); it != mJoystickListeners.end(); ++it) {
		if (!(*it).second->buttonPressed(e, button)) {
			break;
		}
	}

	return true;
}

bool InputManager::buttonReleased(const OIS::JoyStickEvent & e, int button) {
	for (std::map<std::string, OIS::JoyStickListener *>::const_iterator it = mJoystickListeners.begin(); it != mJoystickListeners.end(); ++it) {
		if (!(*it).second->buttonReleased(e, button)) {
			break;
		}
	}

	return true;
}

InputManager * InputManager::getSingletonPtr(void) {
	if (!mInputManager) {
		mInputManager = new InputManager();
	}

	return mInputManager;
}

KeyManager * InputManager::getKeyManager(void) {
	return mKeyManager;
}

void InputManager::setControlsFile(std::string file) {
	mKeyManager->setControlsFile(file);
}

std::string InputManager::getControlsFile() {
	return mKeyManager->getControlsFile();
}

void InputManager::moveMouseTo(int x, int y) {
	OIS::MouseState &mutableMouseState = const_cast<OIS::MouseState &>(mMouse->getMouseState());
	mutableMouseState.X.abs = x;
	mutableMouseState.Y.abs = y;

	mouseMoved(OIS::MouseEvent(mMouse, mutableMouseState));
}

}