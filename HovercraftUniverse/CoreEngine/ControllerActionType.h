#ifndef CONTROLLERACTIONTYPE_H_
#define CONTROLLERACTIONTYPE_H_

namespace HovUni {

namespace ControllerActions{

/**
 * This is an enumeration for the different controller actions.
 *
 * @author Tobias Van Bladel
 */
enum ControllerActionType {
	//Note by Dirk: Do NOT change the values that have explicit constants assigned to them.
	//These values are also used in the scripts and are assumed to be the same.
	INVALID=0, 
	ACCELERATE=1, 
	BRAKE=2, 
	TURNLEFT=3, 
	TURNRIGHT=4,
	RESET=5,


	ACTIONS_END	// shouldn't be used as a real control, just a check for the last entry in the enum
};

/** String representations of the actions */
static std::string actionNames[]={ "Invalid", "Accelerate", "Brake", "Turn Left", "Turn Right", "Reset", };
}

}

#endif
