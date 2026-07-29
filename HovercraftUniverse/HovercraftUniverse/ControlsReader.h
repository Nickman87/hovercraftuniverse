#ifndef CONTROLSREADER_H_
#define CONTROLSREADER_H_

#include "KeyManager.h"
#include "CameraControllerActionType.h"
#include "ControllerActionType.h"

namespace HovUni {

	using namespace ControllerActions;
	using namespace CameraActions;

/**
 * Reads the controls from the control.ini file and registers them with the input manager.
 * TODO: Should be able to write the controls back to the ini file, when they are changed?
 *
 * @author by Tobias Van Bladel
 */

class ControlsReader {
protected:

	/** The key manager */
	KeyManager * mKeyManager;

	/**
	 * Sets all keys from the string (seperated by a comma) for this action.
	 *
	 * @param	action	Action code from the enum.
	 * @param	keys	String with keys, seperated by a comma.
	 * @param	defaultValue	Default key for this action, when no valid keys are given.
	 */
	void setKeys(ControllerActionType action, const std::string keys, std::string defaultValue);

	/**
	 * Sets all keys from the string (seperated by a comma) for this action.
	 *
	 * @param	action	Action code from the enum.
	 * @param	keys	String with keys, seperated by a comma.
	 * @param	defaultValue	Default key for this action, when no valid keys are given.
	 */
	void setCameraKeys(CameraControllerActionType action, std::string keys, std::string defaultValue);


public:

	/**
	 * Constructor.
	 *
	 * @param	keyManager	The key manager where we should add our controls.
	 */
	ControlsReader(KeyManager * keyManager);

	/**
	 * Destructor.
	 */
	~ControlsReader(void);

	/**
	 * Loads the default controls.
	 * Useful when the player has deleted his controls.ini file, or when he wants to reset them.
	 */
	void setDefaultControls();

	/**
	 * Loads the controls from the ini file.
	 */
	void readControls();
};

}

#endif