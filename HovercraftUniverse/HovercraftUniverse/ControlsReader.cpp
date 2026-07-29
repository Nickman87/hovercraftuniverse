#include "ControlsReader.h"
#include "inifile.h"
#include <boost/algorithm/string.hpp>


namespace HovUni {

ControlsReader::ControlsReader(KeyManager * keyManager): mKeyManager(keyManager) {
	// clear current key mapping
	mKeyManager->resetKeyMappings();
	mKeyManager->resetRegisteredActions();
	
	// register actions
	for (int i = 1; i < ACTIONS_END; ++i) {
		mKeyManager->registerAction(i, actionNames[i], "Movement");
	}
}

ControlsReader::~ControlsReader(void) {

}

void ControlsReader::setDefaultControls() {
	// reset current key mapping
	mKeyManager->resetKeyMappings();

	// ACCELERATE
	mKeyManager->setKey(ACCELERATE, OIS::KC_UP, true);
	mKeyManager->setKey(ACCELERATE, OIS::KC_NUMPAD8, true);

	// BRAKE
	mKeyManager->setKey(BRAKE, OIS::KC_DOWN, true);
	mKeyManager->setKey(BRAKE, OIS::KC_NUMPAD2, true);

	// TURN LEFT
	mKeyManager->setKey(TURNLEFT, OIS::KC_LEFT, true);
	mKeyManager->setKey(TURNLEFT, OIS::KC_NUMPAD4, true);

	// TURN RIGHT
	mKeyManager->setKey(TURNRIGHT, OIS::KC_RIGHT, true);
	mKeyManager->setKey(TURNRIGHT, OIS::KC_NUMPAD6, true);

	// RESET
	mKeyManager->setKey(RESET, OIS::KC_R, true);

	// CAMERA
	mKeyManager->setCameraKey(CHANGECAMERA, OIS::KC_C, true);
	mKeyManager->setCameraKey(CHANGECAMERA, OIS::KC_V, true);

	mKeyManager->setCameraKey(THIRD_PERSON_CAMERA, OIS::KC_1, true);
	mKeyManager->setCameraKey(FIRST_PERSON_CAMERA, OIS::KC_2, true);
	mKeyManager->setCameraKey(REAR_VIEW_CAMERA, OIS::KC_3, true);
	mKeyManager->setCameraKey(FREE_CAMERA, OIS::KC_4, true);
	mKeyManager->setCameraKey(MARIO_STYLE_CAMERA, OIS::KC_5, true);

	mKeyManager->setCameraKey(FREE_CAMERA_FWD, OIS::KC_W, true);
	mKeyManager->setCameraKey(FREE_CAMERA_BACK, OIS::KC_S, true);
	mKeyManager->setCameraKey(FREE_CAMERA_LEFT, OIS::KC_A, true);
	mKeyManager->setCameraKey(FREE_CAMERA_RIGHT, OIS::KC_D, true);
	mKeyManager->setCameraKey(FREE_CAMERA_UP, OIS::KC_E, true);
	mKeyManager->setCameraKey(FREE_CAMERA_DOWN, OIS::KC_Q, true);
	mKeyManager->setCameraKey(FREE_CAMERA_ROLL_LEFT, OIS::KC_Z, true);
	mKeyManager->setCameraKey(FREE_CAMERA_ROLL_RIGHT, OIS::KC_C, true);

	// write controls to file
	mKeyManager->writeControlsFile();
}

void ControlsReader::setKeys(ControllerActionType action, const std::string keys, std::string defaultValue) {
	if (keys == "") {
		mKeyManager->setKey(action, defaultValue, true);
	} else {	
		std::vector<std::string> strs;
		boost::split(strs, keys, boost::is_any_of(","));
	
		for (unsigned int i = 0; i < strs.size(); ++i) {
			mKeyManager->setKey(action, strs[i], true);
		}
	}
}

void ControlsReader::setCameraKeys(CameraControllerActionType action, std::string keys, std::string defaultValue) {
	if (keys == "") {
		mKeyManager->setCameraKey(action, defaultValue, true);
	} else {	
		std::vector<std::string> strs;
		boost::split(strs, keys, boost::is_any_of(","));
	
		for (unsigned int i = 0; i < strs.size(); ++i) {
			mKeyManager->setCameraKey(action, strs[i], true);
		}
	}
}

void ControlsReader::readControls() {
	std::string controlINI = mKeyManager->getControlsFile();

	CIniFile reader;
	bool readerSuccess = reader.Load(controlINI);
	if (!readerSuccess) {
		//load defaults
		setDefaultControls();
	} else {

		// reset current key mapping
		mKeyManager->resetKeyMappings();

		setKeys(ACCELERATE, reader.GetKeyValue("Movement", actionNames[ACCELERATE]), "KC_UP");
		setKeys(BRAKE, reader.GetKeyValue("Movement", actionNames[BRAKE]), "KC_DOWN");
		setKeys(TURNLEFT, reader.GetKeyValue("Movement", actionNames[TURNLEFT]), "KC_LEFT");
		setKeys(TURNRIGHT, reader.GetKeyValue("Movement", actionNames[TURNRIGHT]), "KC_RIGHT");
		setKeys(RESET, reader.GetKeyValue("Movement", actionNames[RESET]), "KC_R");

		setCameraKeys(CHANGECAMERA, reader.GetKeyValue("Camera", cameraActionNames[CHANGECAMERA]), "KC_C");
		setCameraKeys(THIRD_PERSON_CAMERA, reader.GetKeyValue("Camera", cameraActionNames[THIRD_PERSON_CAMERA]), "KC_1");
		setCameraKeys(FIRST_PERSON_CAMERA, reader.GetKeyValue("Camera", cameraActionNames[FIRST_PERSON_CAMERA]), "KC_2");
		setCameraKeys(REAR_VIEW_CAMERA, reader.GetKeyValue("Camera", cameraActionNames[REAR_VIEW_CAMERA]), "KC_3");
		setCameraKeys(FREE_CAMERA, reader.GetKeyValue("Camera", cameraActionNames[FREE_CAMERA]), "KC_4");
		setCameraKeys(MARIO_STYLE_CAMERA, reader.GetKeyValue("Camera", cameraActionNames[MARIO_STYLE_CAMERA]), "KC_5");

		setCameraKeys(FREE_CAMERA_FWD, reader.GetKeyValue("FreeCamera", cameraActionNames[FREE_CAMERA_FWD]), "KC_W");
		setCameraKeys(FREE_CAMERA_BACK, reader.GetKeyValue("FreeCamera", cameraActionNames[FREE_CAMERA_BACK]), "KC_S");
		setCameraKeys(FREE_CAMERA_LEFT, reader.GetKeyValue("FreeCamera", cameraActionNames[FREE_CAMERA_LEFT]), "KC_A");
		setCameraKeys(FREE_CAMERA_RIGHT, reader.GetKeyValue("FreeCamera", cameraActionNames[FREE_CAMERA_RIGHT]), "KC_D");
		setCameraKeys(FREE_CAMERA_UP, reader.GetKeyValue("FreeCamera", cameraActionNames[FREE_CAMERA_UP]), "KC_E");
		setCameraKeys(FREE_CAMERA_DOWN, reader.GetKeyValue("FreeCamera", cameraActionNames[FREE_CAMERA_DOWN]), "KC_Q");
		setCameraKeys(FREE_CAMERA_ROLL_LEFT, reader.GetKeyValue("FreeCamera", cameraActionNames[FREE_CAMERA_ROLL_LEFT]), "KC_Z");
		setCameraKeys(FREE_CAMERA_ROLL_RIGHT, reader.GetKeyValue("FreeCamera", cameraActionNames[FREE_CAMERA_ROLL_RIGHT]), "KC_C");
	}

}

}
