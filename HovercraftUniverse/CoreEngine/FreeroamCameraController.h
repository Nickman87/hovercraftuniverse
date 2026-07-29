#ifndef FREEROAMCAMERACONTROLLER_H_
#define FREEROAMCAMERACONTROLLER_H_

#include "Controller.h"
#include <OgreVector3.h>
#include "InputManager.h"
#include <vector>

namespace HovUni {

/**
 * Free roaming Camera controller interface that handles camera movements, based on user controls.
 *
 * @author Kristof Overdulve
 */
class FreeroamCameraController : public Controller, public OIS::KeyListener, public OIS::MouseListener {
private:

	/** The input manager */
	InputManager * mInputManager;

	/** The different states the camera can be moving to */
	bool mMovingLeft;
	bool mMovingForward;
	bool mMovingRight;
	bool mMovingBackward;
	bool mMovingUp;
	bool mMovingDown;
	bool mRollingLeft;
	bool mRollingRight;

	/** The position of the mouse compared to the last position */
	Ogre::Real mMouseRelativeX;
	Ogre::Real mMouseRelativeY;

public:

	/**
	 * Constructor.
	 */
	FreeroamCameraController();

	/**
	 * Destructor
	 */
	virtual ~FreeroamCameraController();

	/**
	 * @see Controller::getEvents()
	 */
	virtual std::vector<ControllerEvent*> getEvents();

	/**
	 * Tells in which direction the camera is to move.
	 *
	 * @return the direction
	 */
	virtual Ogre::Vector3 getDirection();

	/**
	 * Tells the yaw degree that the camera should perform since last frame.
	 *
	 * @return the yaw degree
	 */
	virtual Ogre::Degree getYaw();

	/**
	 * Tells the pitch degree that the camera should perform since last frame.
	 *
	 * @return the pitch degree
	 */
	virtual Ogre::Degree getPitch();

	/**
	 * Tells the roll degree that the camera should perform since last frame.
	 *
	 * @return the roll degree
	 */
	virtual Ogre::Degree getRoll();

	/**
	 * @see KeyListener::keyPressed().
	 */
	bool keyPressed(const OIS::KeyEvent & e);

	/**
	 * @see KeyListener::keyReleased().
	 */
	bool keyReleased(const OIS::KeyEvent & e);

	/**
	 * @see MouseListener::mouseMoved().
	 */
	bool mouseMoved(const OIS::MouseEvent & e);

	/**
	 * @see MouseListener::mousePressed().
	 */
	bool mousePressed(const OIS::MouseEvent & e, OIS::MouseButtonID id);

	/**
	 * @see MouseListener::mouseReleased().
	 */
	bool mouseReleased(const OIS::MouseEvent & e, OIS::MouseButtonID id);
};

}

#endif
