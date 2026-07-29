#ifndef HOVERCRAFTAICONTROLLER_H_
#define HOVERCRAFTAICONTROLLER_H_

#include <string>
#include <vector>
#include <map>
#include "HovercraftController.h"
#include "ScriptWrapper.h"
#include "OgreLuaBindings.h"
#include "ControllerActionType.h"

namespace HovUni {

/**
*	The AI Controller.
*	@author Dirk Delahaye
*/
class HovercraftAIController : public HovercraftController {
	//Provide public accesspoints for AI script.
private:
	/** filename of the Path can be found in level dir*/
	std::string mPathFileName;
	/** Class name*/
	std::string mClassName;
	/** The script object */
	ScriptWrapper* mScript;
	/** The map that maps ControllerActionTypes to integers for the Script */
	std::map<ControllerActions::ControllerActionType, bool> mActionMap;
	
	/** Binds the HovUni::Entity class to LuaBind state */
	void bindEntity(lua_State* L);
public:
	/**
	*	Start the AI Controller with a script.
	*	NOTE: MUST CALL "initialize" function after constructing and setting the controller!
	*	1) Construct the controller
	*	2) setController() in the client
	*	3) AIController.initialize();
	*/
	HovercraftAIController(std::string scriptname, std::string pathfilename);

	/**
	*	Destructor.
	*/
	virtual ~HovercraftAIController();

	/**
	*	Initialization function.
	*	Takes care of bindings and definition of entities.
	*/
	void initialize();

	/**
	*	Initiate or stop an action (the AI presses or releases a key)
	*	@param	action	The action (ControllerActionType value).
	*	@pararm	state	The current state (on or off: true or false)
	*/
	void setAction(const int action, const bool state);

	/**
	*	Get the current state of an action.
	*	@param	action	The action (ControllerActionType value)
	*	@return	The current state (true for on, false for off)
	*/
	bool getAction(const int action);

	/**
	*	Provides a logging function for the Lua Script.
	*/
	void luaLog(const std::string message);

	/**
	*	Provides a way for the AI script to retrieve current boosting status of the hovercraft
	*/
	bool getBoosting();

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
	 * @see Controller::getEvents()
	 */
	std::vector<ControllerEvent*> getEvents();
};

}

#endif //HovercraftAIController_H_