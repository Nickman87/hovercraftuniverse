#ifndef RACESTATELISTENER_H
#define RACESTATELISTENER_H

#include "RacePlayer.h"
#include "RaceState.h"

namespace HovUni {

/**
 * A listener for the race state
 *
 * @author Olivier Berghmans
 */
class RaceStateListener {
public:
	/**
	 * The state of the race changed
	 *
	 * @param state the new state
	 */
	virtual void onStateChange(RaceState::States state) = 0;

	/**
	 * The player changed position
	 *
	 * @param player the player
	 */
	virtual void onPositionChange(RacePlayer* player) = 0;

	/**
	 * The player finished
	 *
	 * @param player the player
	 */
	virtual void onFinish(RacePlayer* player) = 0;

};

}

#endif
