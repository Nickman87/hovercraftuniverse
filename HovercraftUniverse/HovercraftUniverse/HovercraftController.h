#ifndef HOVERCRAFTCONTROLLER_H
#define HOVERCRAFTCONTROLLER_H

#include "Controller.h"
#include <OgreVector3.h>
#include "BasicEntityEvent.h"
#include "Timing.h"

namespace HovUni {

/**
 * Dummy hovercraft controller interface that specifies which actions must be under control.
 *
 * @author Kristof Overdulve, Olivier Berghmans, Dirk Delahaye
 */
class HovercraftController : public Controller {
protected:
	/** The last sent moving state */
	BasicEntityEvent mLast;

	/** A timer */
	Timing mTimer;

	//Temp
	__int64 mPreviousTime;

public:
	/**
	 * Default constructor
	 */
	HovercraftController();

	/**
	 * Destructor
	 */
	virtual ~HovercraftController();

	/**
	 * @see Controller::getEvents()
	 */
	virtual std::vector<ControllerEvent*> getEvents();

	/**
	 * Check whether the entity should move forward
	 *
	 * @return true if the entity should move forward
	 */
	virtual bool moveForward() = 0;

	/**
	 * Check whether the entity should move backward
	 *
	 * @return true if the entity should move backward
	 */
	virtual bool moveBackward() = 0;

	/**
	 * Check whether the entity should turn left
	 *
	 * @return true if the entity should turn left
	 */
	virtual bool turnLeft() = 0;

	/**
	 * Check whether the entity should turn right
	 *
	 * @return true if the entity should turn right
	 */
	virtual bool turnRight() = 0;

	/**
	 * Check whether the entity should be reseted
	 *
	 * @return true if the entity should be reseted
	 */
	virtual bool reset() = 0;

};

}

#endif
