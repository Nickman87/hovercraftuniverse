#ifndef HOVERCRAFTPLAYERCONTROLLER_H_
#define HOVERCRAFTPLAYERCONTROLLER_H_

#include "HovercraftController.h"
#include <OgreVector3.h>
#include "InputManager.h"

namespace HovUni {

/**
 * Hovercraft player controller that listens to controls and feeds it to the dummy hovercraft.
 *
 * @author Kristof Overdulve, edited by Tobias Van Bladel
 */
class HovercraftPlayerController : public HovercraftController, public OIS::KeyListener {
protected:
	
	/** The input manager */
	InputManager * mInputManager;

	/** The different states the hovercraft can be in */
	bool mMovingLeft;
	bool mMovingForward;
	bool mMovingRight;
	bool mMovingBackward;
	bool mReset;


public:
	/**
	 * Constructor.
	 */
	HovercraftPlayerController(void);

	/**
	 * Destructor.
	 */
	virtual ~HovercraftPlayerController(void);

	/**
	 * @see HovercraftController::moveForward()
	 */
	bool moveForward();

	/**
	 * @see HovercraftController::moveBackward()
	 */
	bool moveBackward();

	/**
	 * @see HovercraftController::turnLeft()
	 */
	bool turnLeft();

	/**
	 * @see HovercraftController::turnRight()
	 */
	bool turnRight();

	/**
	 * @see HovercraftController::reset()
	 */
	bool reset();

	/**
	 * @see KeyListener::keyPressed().
	 */
	bool keyPressed(const OIS::KeyEvent & e);

	/**
	 * @see KeyListener::keyReleased().
	 */
	bool keyReleased(const OIS::KeyEvent & e);
};

}

#endif

