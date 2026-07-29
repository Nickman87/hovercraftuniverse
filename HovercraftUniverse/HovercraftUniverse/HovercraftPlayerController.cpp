#include "HovercraftPlayerController.h"
#include "ControllerActionType.h"
#include <OgreLogManager.h>

namespace HovUni {

HovercraftPlayerController::HovercraftPlayerController(void) : mMovingLeft(false), mMovingForward(false),
		mMovingRight(false), mMovingBackward(false), mReset(false) {
	// Fetch input manager object
	mInputManager = InputManager::getSingletonPtr();
	mInputManager->addKeyListener(this, "HovercraftPlayerController");
}

HovercraftPlayerController::~HovercraftPlayerController(void) {
	// Remove key listener from controller
	mInputManager->removeKeyListener("HovercraftPlayerController");
}

bool HovercraftPlayerController::moveForward() {
	return mMovingForward;
}

bool HovercraftPlayerController::moveBackward() {
	return mMovingBackward;
}

bool HovercraftPlayerController::turnLeft() {
	return mMovingLeft;
}

bool HovercraftPlayerController::turnRight() {
	return mMovingRight;
}

bool HovercraftPlayerController::reset() {
	return mReset;
}

bool HovercraftPlayerController::keyPressed(const OIS::KeyEvent & e) { 
	
	// TODO: this is a temporary hack, should be removed after Nick toggles this in the gamestate somewhere...
	if (e.key == OIS::KC_B) {
		mInputManager->getKeyManager()->toggleActivity();
	}

	// Process possibly resulting move
	int action = mInputManager->getKeyManager()->getAction(e.key);
	switch (action) {
		case ControllerActions::ACCELERATE:
			mMovingForward = true;
			break;
		case ControllerActions::BRAKE:
			mMovingBackward = true;
			break;
		case ControllerActions::TURNLEFT:
			mMovingLeft = true;
			break;
		case ControllerActions::TURNRIGHT:
			mMovingRight = true;
			break;
		case ControllerActions::RESET:
			mReset = true;
			break;
		default:
			break;
	}
	
	// Succes
	return true; 
}

bool HovercraftPlayerController::keyReleased(const OIS::KeyEvent & e) { 
	int action = mInputManager->getKeyManager()->getAction(e.key);
	// Clear movement for that key
	switch (action) {
		case ControllerActions::ACCELERATE:
			mMovingForward = false;
			break;
		case ControllerActions::BRAKE:
			mMovingBackward = false;
			break;
		case ControllerActions::TURNLEFT:
			mMovingLeft = false;
			break;
		case ControllerActions::TURNRIGHT:
			mMovingRight = false;
			break;
		case ControllerActions::RESET:
			mReset = false;
			break;
		default:
			break;
	}

	// Succes
	return true; 
}

}