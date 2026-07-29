#include "RaceCamera.h"
#include "CameraControllerActionType.h"
#include <OgreLogManager.h>
#include "CameraSpring.h"
#include "RepresentationManager.h"

class Application;

namespace HovUni {

RaceCamera::RaceCamera(Ogre::SceneManager * sceneMgr, int ID) : mSceneMgr(sceneMgr), mID(ID), mCurrCamViewpoint(ThirdPerson) {
	// Fetch input manager object and register controls
	mInputManager = InputManager::getSingletonPtr();
	mInputManager->addKeyListener(this, "RaceCamera");

	// Create camera controllers
	mFreeroamCameraController = new FreeroamCameraController();

	// Create camera for this game view
	mCamera = mSceneMgr->createCamera("Camera" + mID);
	mCamera->setNearClipDistance(0.1f);
	mCamera->setFarClipDistance(30000.0f);


	//////////////////////////
	//	NEW CAMERA SYSTEM	//
	//////////////////////////
	/*
	mCameraCS = new CCS::CameraControlSystem(mSceneMgr, "CameraControlSystem", mCamera);

	CCS::ChaseCameraMode* chaseCam;
    chaseCam = new CCS::ChaseCameraMode(mCameraCS, Ogre::Vector3(0,20.0f,-200.0f));
    mCameraCS->registerCameraMode("Chase",chaseCam);

	//mCameraCS->setCameraTarget(headNode);
    mCameraCS->setCurrentCameraMode(chaseCam);
	*/
	/////////// END //////////
}

void RaceCamera::reinitialize() {
	// Create 3rd person view camera
	m3rdPersonViewpointNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(mCamera->getName() + "3rdPersonNode");
	m3rdPersonViewpointNode = m3rdPersonViewpointNode->createChildSceneNode(m3rdPersonViewpointNode->getName() + "Pitch");
	//Init the camera now to avoid any chance of null pointers
	CameraSpring::getInstance()->initCameraSpring(mCamera->getPosition(),//Cameras current position
		Ogre::Vector3::ZERO);//Offset is 100 units behind(-) the player

	// Create mario style camera
	mMarioStyleViewpointNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(mCamera->getName() + "MarioNode");
	mMarioStyleViewpointNode = mMarioStyleViewpointNode->createChildSceneNode(m3rdPersonViewpointNode->getName() + "Pitch");
	//Init the camera now to avoid any chance of null pointers
	CameraSpring::getInstance()->initCameraSpring(mCamera->getPosition(),//Cameras current position
		Ogre::Vector3::ZERO);//Offset is 100 units behind(-) the player

	// Create 1st person view camera
	m1stPersonViewpointNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(mCamera->getName() + "1stPersonNode");
	m1stPersonViewpointNode = m1stPersonViewpointNode->createChildSceneNode(m1stPersonViewpointNode->getName() + "Pitch");

	// Create rear view camera
	mRearViewpointNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(mCamera->getName() + "RearPersonNode"); 
	mRearViewpointNode = mRearViewpointNode->createChildSceneNode(mRearViewpointNode->getName() + "Pitch");

	// Create free roam camera
	mFreeRoamViewpointNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(mCamera->getName() + "FreeRoamNode", 
		Ogre::Vector3(-40, 20, 40));
	mFreeRoamViewpointNode->yaw(Ogre::Degree(-45));
	mFreeRoamViewpointNode = mFreeRoamViewpointNode->createChildSceneNode(mFreeRoamViewpointNode->getName() + "Pitch");

	// Set default camera and controller
	mActiveViewpointNode = m3rdPersonViewpointNode;
	mActiveViewpointNode->attachObject(mCamera);
	mCurrCamViewpoint = ThirdPerson;
}

RaceCamera::~RaceCamera() {
	// Delete camera controller
	delete mFreeroamCameraController;
	mFreeroamCameraController = 0;

	// Unregister controls
	mInputManager->removeKeyListener("RaceCamera");
}

void RaceCamera::setFreeroam(Ogre::Vector3 pos, Ogre::Quaternion orientation) {
	Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "Position is now: " << pos << "\n";
	mFreeRoamViewpointNode->setPosition(pos);
	mFreeRoamViewpointNode->setOrientation(orientation);
}

void RaceCamera::setFreeroam(Ogre::Vector3 pos, Ogre::Quaternion orientation, Ogre::Vector3 dir) {
	setFreeroam(pos, orientation);
	mFreeRoamViewpointNode->setDirection(dir);
}

bool RaceCamera::keyPressed(const OIS::KeyEvent & e) { 
	CameraActions::CameraControllerActionType action = mInputManager->getKeyManager()->getCameraAction(e.key);
	EntityRepresentation* trackedEntity = RepresentationManager::getSingletonPtr()->getTrackedEntityRepresentation();
	// TODO Set the camera controller
	bool freeroamSwitch = false;
	bool wasCameraAction = true;
	
	switch (action) {
	case CameraActions::CHANGECAMERA:
		// Switch to next camera
		if (mCurrCamViewpoint == MarioStyle) {
			mCurrCamViewpoint = ThirdPerson;
		} else {
			mCurrCamViewpoint = CameraViewpoint(mCurrCamViewpoint + 1);

			if (mCurrCamViewpoint == FreeRoam) {
				freeroamSwitch = true;
			}
		}
		break;
	case CameraActions::THIRD_PERSON_CAMERA:
		// Switch to 3rd person
		mCurrCamViewpoint = ThirdPerson;
		break;
	case CameraActions::MARIO_STYLE_CAMERA:
		// Switch to mario style
		mCurrCamViewpoint = MarioStyle;
		break;
	case CameraActions::FIRST_PERSON_CAMERA:
		// Switch to 1st person
		mCurrCamViewpoint = FirstPerson;
		break;
	case CameraActions::REAR_VIEW_CAMERA:
		// Switch to rear view
		mCurrCamViewpoint = RearView;
		break;
	case CameraActions::FREE_CAMERA:
		// Switch to free roaming
		mCurrCamViewpoint = FreeRoam;
		freeroamSwitch = true;
		break;
	default:
		// Do nothing
		wasCameraAction = false;
		break;
	}

	if (wasCameraAction) {
		mActiveViewpointNode->detachObject(mCamera);

		if (mCurrCamViewpoint == ThirdPerson) {
			mActiveViewpointNode = m3rdPersonViewpointNode;
		} else if (mCurrCamViewpoint == MarioStyle) {
			mActiveViewpointNode = mMarioStyleViewpointNode;
		} else if (mCurrCamViewpoint == FirstPerson) {
			mActiveViewpointNode = m1stPersonViewpointNode;
		} else if (mCurrCamViewpoint == RearView) {
			mActiveViewpointNode = mRearViewpointNode;
		} else if (mCurrCamViewpoint == FreeRoam) {
			mActiveViewpointNode = mFreeRoamViewpointNode;
		}
		mActiveViewpointNode->attachObject(mCamera);
	}

	if (freeroamSwitch) {
		//Initialise the freeroam camera
		if (trackedEntity != 0) {
			Ogre::Vector3 position = trackedEntity->getEntity()->getPosition() - (trackedEntity->getEntity()->getOrientation() * 50) + (trackedEntity->getEntity()->getUpVector() * 40);
			mActiveViewpointNode->setPosition(position);
			mActiveViewpointNode->lookAt(trackedEntity->getEntity()->getPosition(), Ogre::Node::TS_WORLD);
			//mCamera->setPosition(position);
			//mCamera->lookAt(trackedEntity->getEntity()->getPosition());
		}
	}

	// Succes
	return true; 
}

bool RaceCamera::keyReleased(const OIS::KeyEvent & e) { 
	return true; 
}

void RaceCamera::update(Ogre::Real timeSinceLastFrame) {
	Ogre::Vector3 oldPosition;
	Ogre::Vector3 newPosition;
	Ogre::Vector3 positionCam;
	Ogre::Quaternion back;
	Ogre::SceneNode* trackedNode = 0;
	EntityRepresentation* trackedEntity = RepresentationManager::getSingletonPtr()->getTrackedEntityRepresentation();

	if (trackedEntity != 0) {
		Entity* currEntity = trackedEntity->getEntity();

		switch (mCurrCamViewpoint) {
		case ThirdPerson:
			newPosition = currEntity->getPosition() - (currEntity->getOrientation() * 20) + (currEntity->getUpVector() * 10);
			
			positionCam = CameraSpring::getInstance()->updateCameraSpring(mCamera->getPosition(), newPosition);
			mCamera->setFixedYawAxis(true, currEntity->getUpVector()); // Comment this line for super mario galaxy style!
			mCamera->setPosition(positionCam);

			//positionCam = CameraSpring::getInstance()->updateCameraSpring(mActiveViewpointNode->getPosition(), newPosition); //Smooth tracking
			//positionCam = newPosition; //Hard tracking
			//mActiveViewpointNode->setPosition(positionCam);
			//mActiveViewpointNode->setOrientation(currEntity->getQuaternion());

			mCamera->lookAt(currEntity->getSmoothPosition() + currEntity->getUpVector() * 5);
			//mActiveViewpointNode->lookAt(currEntity->getSmoothPosition() + currEntity->getUpVector() * 5, Ogre::Node::TS_WORLD);

			//mCamera->setPosition(newPosition);
			//mCamera->lookAt(mObjectTrackCameraController->getPosition());
			//turn the camera slightly to the tracked entity
			//mCamera->setOrientation(mObjectTrackCameraController->getOrientation());
			//mCamera->pitch(Ogre::Degree(-15.0f));

			/// OLD SHIT
			//std::cout << "UP vector :: " << mObjectTrackCameraController->getUpVector() << std::endl;
			//oldPosition = mActiveViewpointNode->getPosition();
			// Determine position camera
			//newPosition = mObjectTrackCameraController->getPosition() - (mObjectTrackCameraController->getDirection() * 20) + (mObjectTrackCameraController->getUpVector() * 10);
			//positionCam = oldPosition + (newPosition - oldPosition) * 0.10f;
			//std::cout << oldPosition << "   " << newPosition << "   " << positionCam << std::endl;
			// Set position and direction to look at
			//mActiveViewpointNode->setPosition(positionCam);
			//mActiveViewpointNode->setOrientation(mObjectTrackCameraController->getOrientation());
			//turn the camera slightly to the tracked entity
			//mActiveViewpointNode->pitch(Ogre::Degree(-15.0f), Ogre::Node::TS_LOCAL);
			
			break;
		case MarioStyle:
			newPosition = currEntity->getPosition() - (currEntity->getOrientation() * 20) + (currEntity->getUpVector() * 10);
			positionCam = CameraSpring::getInstance()->updateCameraSpring(mCamera->getPosition(), newPosition);
			mCamera->setPosition(positionCam);
			mCamera->lookAt(currEntity->getSmoothPosition() + currEntity->getUpVector() * 5);
			
			break;
		case FirstPerson:
			// Determine position camera
			positionCam = currEntity->getPosition() + currEntity->getOrientation();

			// Set position and direction to look at
			mCamera->setPosition(positionCam);
			mCamera->setOrientation(currEntity->getQuaternion());
			mCamera->pitch(Ogre::Degree(-5.0f));

			//mActiveViewpointNode->setPosition(positionCam);
			//mActiveViewpointNode->setOrientation(currEntity->getQuaternion());
			//mActiveViewpointNode->pitch(Ogre::Degree(-5.0f));

			break;
		case RearView:
			// Determine position camera
			positionCam = currEntity->getPosition() - currEntity->getOrientation();
			// turn the camera 180 degrees around the up vector
			back = Ogre::Quaternion(Ogre::Degree(180), currEntity->getUpVector());

			// Set position and direction to look at
			mCamera->setPosition(positionCam);
			mCamera->setOrientation(back * currEntity->getQuaternion());


			//mActiveViewpointNode->setPosition(positionCam);
			//mActiveViewpointNode->setOrientation(back * currEntity->getQuaternion());

			break;
		case FreeRoam:
			mCamera->setFixedYawAxis(false, Ogre::Vector3::UNIT_Y);
			// Get input from free roaming controller and apply
			mCamera->yaw(mFreeroamCameraController->getYaw());
			mCamera->pitch(mFreeroamCameraController->getPitch());
			mCamera->roll(mFreeroamCameraController->getRoll());
			mCamera->setPosition(mCamera->getPosition() +  (mCamera->getOrientation() * mFreeroamCameraController->getDirection()) * (timeSinceLastFrame * 100));

			//mActiveViewpointNode->yaw(mFreeroamCameraController->getYaw());
			//mActiveViewpointNode->pitch(mFreeroamCameraController->getPitch());
			//mActiveViewpointNode->setPosition(mActiveViewpointNode->getPosition() +  (mActiveViewpointNode->getOrientation() * mFreeroamCameraController->getDirection()) * (timeSinceLastFrame * 100));

			break;
		default:
			// Impossible
			break;
		}
	}
}

}