#ifndef KEYMANAGER_H_
#define KEYMANAGER_H_

#include <OIS.h>
#include <vector>
#include "CameraControllerActionType.h"

namespace HovUni {

/**
 * This class maps keys to their actions.
 *
 * @author Tobias Van Bladel
 */

class KeyManager {
private:
	/** Mapping with keycodes to actions */
	std::map<OIS::KeyCode, int> mKeyMapping;

	/** Mapping with the camera controls (game-independent) */
	std::map<OIS::KeyCode, CameraActions::CameraControllerActionType> mCameraMapping;

	/** Mapping with actions to their name */
	std::map<int, std::string> mActionMapping;

	/** Mapping with actions to their type */
	std::map<int, std::string> mActionTypeMapping;

	/** Mapping with key names to their OIS keycode */
	std::map<std::string, OIS::KeyCode> mKeyCodeMapping;

	/** The config file with the controls */
	std::string mControlsFile;

	/** The activity of the controls. Should be true in-game, and false in the menu. */
	bool mActive;

	/**
	 * Register the keycodes.
	 */
	void registerKeyCodes();

public:
	/**
	 * Constructor
	 */
	KeyManager(void);

	/**
	 * Destructor
	 */
	virtual ~KeyManager(void);

	//TODO: is OIS::KeyCode voldoende? Werkt dit ook voor bv controllers?

	
	/**
	 * Resets all key mappings, except the registered actions.
	 */
	void resetKeyMappings();

	/**
	 * Resets all registered actions.
	 */
	void resetRegisteredActions();

	/**
	 * Write the current key mappings to the controls file.
	 */
	void writeControlsFile();

	/**
	 * Register an action.
	 *
	 * @param	action	Action you want to register.
	 * @param	name	Name of the action.
	 * @param	type	Subtype of the action
	 */
	void registerAction(const int action, std::string name, std::string type);

	/** 
	 * Get the key for a certain action.
	 *
	 * @param	action	Action you want the keys for.
	 * @return	List with all the keys for this action (could be empty!).
	 */
	const std::vector<OIS::KeyCode> getKeys(const int action);

	/** 
	 * Change the key for this action.
	 * This will overwrite the controls file, unless initial is true.
	 *
	 * @param	action	Action you want to change the key of.
	 * @param	key		New key for this action.
	 * @param	initial	Is this the initial key setting?
	 */
	void setKey(const int action, const OIS::KeyCode key, bool initial = false);

	/** 
	 * Change the key for this action.
	 * This will overwrite the controls file, unless initial is true.
	 *
	 * @param	action	Action you want to change the key of.
	 * @param	key		Name of the key for this action.
	 * @param	initial	Is this the initial key setting?
	 */
	void setKey(const int action, std::string key, bool initial = false);

	/**
	 * Gets the action associated with this key.
	 *
	 * @param	key		The key for which you want the registered action.
	 * @return	The action associated with this key.
	 */
	const int getAction(const OIS::KeyCode key);

	/**
	 * Get all registered actions (as int code).
	 *
	 * @return	All registered user actions.
	 */
	const std::vector<int> getRegisteredActions();

	/** 
	 * Get the string representation of the action.
	 *
	 * @param	action	Action for which you want the name.
	 * @return	The name of this action, if it is registered.
	 */
	const std::string getActionName(const int action);

	/** 
	 * Get the type of the action.
	 *
	 * @param	action	Action for which you want the name.
	 * @return	The type of this action, if it is registered.
	 */
	const std::string getActionType(const int action);

	/** 
	 * Get the key for a certain camera action.
	 *
	 * @param	action	Action you want the keys for.
	 * @return	List with all the keys for this camera action (could be empty!).
	 */
	const std::vector<OIS::KeyCode> getCameraKeys(const CameraActions::CameraControllerActionType action);

	/** 
	 * Change the key for this camera action.
	 * This will overwrite the controls file, unless initial is true.
	 *
	 * @param	action	Action you want to change the key of.
	 * @param	key		New key for this camera action.
	 * @param	initial	Is this the initial key setting?
	 */
	void setCameraKey(const CameraActions::CameraControllerActionType action, const OIS::KeyCode key, bool initial = false);

	/** 
	 * Change the key for this camera action.
	 * This will overwrite the controls file, unless initial is true.
	 *
	 * @param	action	Action you want to change the key of.
	 * @param	key		Name of the key for this camera action.
	 * @param	initial	Is this the initial key setting?
	 */
	void setCameraKey(const CameraActions::CameraControllerActionType action, std::string key, bool initial = false);

	/**
	 * Gets the cameraaction associated with this key.
	 *
	 * @param	key		The key for which you want the registered action.
	 * @return	The action associated with this key.
	 */
	const CameraActions::CameraControllerActionType getCameraAction(const OIS::KeyCode key);

	/** 
	 * Get the string representation of the camera action.
	 *
	 * @param	action	Action for which you want the name.
	 * @return	The name of this camera action.
	 */
	const std::string getCameraName(const CameraActions::CameraControllerActionType action);

	/**
	 * Get the key code of this key name.
	 *
	 * @param	name	Name of the key.
	 * @return	The key code of this key name.
	 */
	OIS::KeyCode getKeyCode(std::string name);

	/**
	 * Get the key name of this key code.
	 *
	 * @param	key	Key code of the key.
	 * @return	The key name of this key code.
	 */
	std::string getKeyName(OIS::KeyCode key);

	/**
	 * Set the location of the controls ini file
	 *
	 * @param	file	Location of Controls.ini
	 */
	void setControlsFile(std::string file);

	/**
	 * Get the location of the controls ini file
	 *
	 * @return	file	Location of Controls.ini
	 */
	std::string getControlsFile();

	/**
	 * Toggle controls activity.
	 */
	void toggleActivity();

	/**
	 * Set controls active.
	 */
	void setActive();
	
	/**
	 * Set controls inactive.
	 */
	void setInactive();

	/**
	 * Are controls active?
	 *
	 * @return True of the controls are active, false otherwise.
	 */
	bool isActive();

};

}

#endif