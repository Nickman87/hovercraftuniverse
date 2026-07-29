#include "FreeroamCameraController.h"
#include "CameraControllerActionType.h"
#include <OgreLogManager.h>

namespace HovUni {

FreeroamCameraController::FreeroamCameraController() : mMovingLeft(false), mMovingForward(false),
		mMovingRight(false), mMovingBackward(false), mMovingUp(false), mMovingDown(false), mMouseRelativeX(0.0),
		mMouseRelativeY(0.0) {
	// Fetch input manager object and add listeners
	mInputManager = InputManager::getSingletonPtr();
	mInputManager->addKeyListener(this, "FreeroamCameraController");
	mInputManager->addMouseListener(this, "FreeroamCameraController");
}

FreeroamCameraController::~FreeroamCameraController() {
	// Remove listeners from controller
	mInputManager->removeKeyListener("FreeroamCameraController");
	mInputManager->removeMouseListener("FreeroamCameraController");
}

std::vector<ControllerEvent*> FreeroamCameraController::getEvents() {
	// These events are really not interesting for network transfer and thus, this method should not be called
	assert (false);

	// Return empty vector
	return std::vector<ControllerEvent*>();
}

Ogre::Vector3 FreeroamCameraController::getDirection() {
	// Process moves
	Ogre::Vector3 accumulatedDirection = Ogre::Vector3::ZERO;
	if (mMovingLeft) { accumulatedDirection += Ogre::Vector3::NEGATIVE_UNIT_X; }
	if (mMovingForward) { accumulatedDirection += Ogre::Vector3::NEGATIVE_UNIT_Z; }
	if (mMovingRight) { accumulatedDirection += Ogre::Vector3::UNIT_X; }
	if (mMovingBackward) { accumulatedDirection += Ogre::Vector3::UNIT_Z; }
	if (mMovingUp) { accumulatedDirection += Ogre::Vector3::UNIT_Y; }
	if (mMovingDown) { accumulatedDirection += Ogre::Vector3::NEGATIVE_UNIT_Y; }

	// No movement
	return accumulatedDirection.normalisedCopy();
}

Ogre::Degree FreeroamCameraController::getYaw() {
	// Relative X to return
	Ogre::Real mouseRelativeX = mMouseRelativeX;

	// Clear relative X
	mMouseRelativeX = 0.0;

	// Return yaw
	return Ogre::Degree((Ogre::Real) -0.13 * mouseRelativeX);
}

Ogre::Degree FreeroamCameraController::getPitch() {
	// Relative Y to return
	Ogre::Real mouseRelativeY = mMouseRelativeY;

	// Clear relative Y
	mMouseRelativeY = 0.0;

	// Return pitch
	return Ogre::Degree((Ogre::Real) -0.13 * mouseRelativeY);
}

Ogre::Degree FreeroamCameraController::getRoll() {
	Ogre::Real rolling = Ogre::Real(0.0f);
	if (mRollingLeft) { 
		rolling -= 1.0f;
	}
	if (mRollingRight) { 
		rolling += 1.0f;
	}

	// Return pitch
	return Ogre::Degree(rolling);
}

bool FreeroamCameraController::keyPressed(const OIS::KeyEvent & e) { 
	CameraActions::CameraControllerActionType action = mInputManager->getKeyManager()->getCameraAction(e.key);
	// Process possibly resulting move
	switch (action) {
		case CameraActions::FREE_CAMERA_FWD:
			mMovingForward = true;
			break;
		case CameraActions::FREE_CAMERA_BACK:
			mMovingBackward = true;
			break;
		case CameraActions::FREE_CAMERA_LEFT:
			mMovingLeft = true;
			break;
		case CameraActions::FREE_CAMERA_RIGHT:
			mMovingRight = true;
			break;
		case CameraActions::FREE_CAMERA_UP:
			mMovingUp = true;
			break;
		case CameraActions::FREE_CAMERA_DOWN:
			mMovingDown = true;
			break;
		case CameraActions::FREE_CAMERA_ROLL_LEFT:
			mRollingLeft = true;
			break;
		case CameraActions::FREE_CAMERA_ROLL_RIGHT:
			mRollingRight = true;
			break;
		default:
			break;
	}
	
	// Succes
	return true; 
}

bool FreeroamCameraController::keyReleased(const OIS::KeyEvent & e) { 
	CameraActions::CameraControllerActionType action = mInputManager->getKeyManager()->getCameraAction(e.key);
	// Clear movement for that key
	switch (action) {
		case CameraActions::FREE_CAMERA_FWD:
			mMovingForward = false;
			break;
		case CameraActions::FREE_CAMERA_BACK:
			mMovingBackward = false;
			break;
		case CameraActions::FREE_CAMERA_LEFT:
			mMovingLeft = false;
			break;
		case CameraActions::FREE_CAMERA_RIGHT:
			mMovingRight = false;
			break;
		case CameraActions::FREE_CAMERA_UP:
			mMovingUp = false;
			break;
		case CameraActions::FREE_CAMERA_DOWN:
			mMovingDown = false;
			break;
		case CameraActions::FREE_CAMERA_ROLL_LEFT:
			mRollingLeft = false;
			break;
		case CameraActions::FREE_CAMERA_ROLL_RIGHT:
			mRollingRight = false;
			break;
		default:
			break;
	}

	// Succes
	return true; 
}

bool FreeroamCameraController::mouseMoved(const OIS::MouseEvent & e) { 
	// Handle mouse movements of camera
	mMouseRelativeX += mInputManager->getMouse()->getMouseState().X.rel;
	mMouseRelativeY += mInputManager->getMouse()->getMouseState().Y.rel;

	return true; 
}

bool FreeroamCameraController::mousePressed(const OIS::MouseEvent & e, OIS::MouseButtonID id) { 
	return true;
}
bool FreeroamCameraController::mouseReleased(const OIS::MouseEvent & e, OIS::MouseButtonID id) { 
	return true; 
}

}