#ifndef RACECAMERA_H_
#define RACECAMERA_H_

#include <OgreSceneManager.h>
#include "InputManager.h"
#include "FreeroamCameraController.h"

/*
#include <CCSCameraControlSystem.h>
#include "CCSBasicCameraModes.h"
#include "CCSFreeCameraMode.h"
#include "CCSOrbitalCameraMode.h"
*/
namespace HovUni {

/**
 * Defines a racing camera to use in racing games. This camera has a 3rd person front view, a first person front view,
 * a rear view and a free roaming view.
 *
 * @author Kristof Overdulve
 */
class RaceCamera : public OIS::KeyListener {
private:

	/** The Ogre scene manager */
	Ogre::SceneManager * mSceneMgr;

	/** The camera of the view */
	Ogre::Camera * mCamera;

	/** The ID of the camera */
	int mID;

	/** The 3d person viewpoint of the camera */
	Ogre::SceneNode * m3rdPersonViewpointNode;

	/** The 3d person mario style camera */
	Ogre::SceneNode * mMarioStyleViewpointNode;

	/** The first person viewpoint of the camera */
	Ogre::SceneNode * m1stPersonViewpointNode;

	/** The rear viewpoint of the camera */
	Ogre::SceneNode * mRearViewpointNode;

	/** The free roaming viewpoint of the camera */
	Ogre::SceneNode * mFreeRoamViewpointNode;

	/** The currently active camera viewpoint */
	Ogre::SceneNode * mActiveViewpointNode;

	/**
	 * The camera viewpoints
	 */
	enum CameraViewpoint {
		ThirdPerson = 0,
		FirstPerson = 1,
		RearView = 2,
		FreeRoam = 3,
		MarioStyle = 4
	};

	/** The camera currently active */
	CameraViewpoint mCurrCamViewpoint;

	/** The input manager */
	InputManager * mInputManager;

	/** The free roaming camera controller */
	FreeroamCameraController * mFreeroamCameraController;

	//////////////////////////
	//	NEW CAMERA SYSTEM	//
	//////////////////////////
	//CCS::CameraControlSystem* mCameraCS;
	/////////// END //////////

public:

	/**
	 * Constructor.
	 *
	 * @param sceneMgr the Ogre scene manager
	 * @param the ID of the camera created here
	 */
	RaceCamera(Ogre::SceneManager * sceneMgr, int ID);

	/**
	 * Destructor.
	 */
	~RaceCamera(void);

	/**
	 * Reinitialize the camera.
	 */
	void reinitialize();

	/**
	 * Sets the free roaming position and orientation.
	 *
	 * @param pos the position of the freeroam camera
	 * @param orientation the orientation of the camera
	 */
	void setFreeroam(Ogre::Vector3 pos, Ogre::Quaternion orientation);

	/**
	 * Sets the free roaming position and orientation.
	 *
	 * @param pos the position of the freeroam camera
	 * @param orientation the orientation of the camera
	 * @param dir the direction to which the camera points
	 */
	void setFreeroam(Ogre::Vector3 pos, Ogre::Quaternion orientation, Ogre::Vector3 dir);

	/**
	 * @see KeyListener::keyPressed().
	 */
	bool keyPressed(const OIS::KeyEvent & e);

	/**
	 * @see KeyListener::keyReleased().
	 */
	bool keyReleased(const OIS::KeyEvent & e);

	/**
	 * Updates the camera.
	 *
	 * @param timeSinceLastFrame the time since last frame was drawn
	 */
	void update(Ogre::Real timeSinceLastFrame);

	/**
	 * Returns the camera associated with this game view.
	 *
	 * @return the camera
	 */
	Ogre::Camera * getCamera() { return mCamera; }

	void setPosition(const Ogre::Vector3& position, const Ogre::Vector3& direction) {
		mCamera->setPosition(position);
		mCamera->setDirection(direction);
	}

};

}

#endif