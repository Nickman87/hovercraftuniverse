#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <vector>
#include "ControllerEvent.h"

namespace HovUni {

class Entity;

/**
 * Provides controls for a game character. This interface is polled by entities in order to check
 * what the entity should do. These controllers could be player controls, AI controls, game controls, 
 * ...
 *
 * @author Kristof Overdulve & Olivier Berghmans
 */
class Controller {

private:
	
	/** The entity to which this controller refers */
	Entity * mEntity;

public:

	/**
	 * Constructor.
	 */
	Controller() { 
		// Set entity to null
		mEntity = 0;
	}

	/**
	 * Destructor
	 */
	virtual ~Controller() {}

	/**
	 * Sets the entity to which this controller refers.
	 * 
	 * @param entity The entity to which this controller refers
	 */
	void setEntity(Entity * const entity) { 
		mEntity = entity; 
	}

	/**
	 * Returns the entity to which this controller refers.
	 *
	 * @return the entity or null if there is no entity assigned yet
	 */
	Entity * const getEntity() { 
		return mEntity;
	}

	/**
	 * Poll for a list of controller events
	 *
	 * @return the list of events
	 */
	virtual std::vector<ControllerEvent*> getEvents() = 0;

};

}

#endif