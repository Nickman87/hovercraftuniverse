#include "KeyManager.h"
#include "inifile.h"
#include <utility>
#include <algorithm>

namespace HovUni {

KeyManager::KeyManager(void): mActive(true) {
	registerKeyCodes();
}

KeyManager::~KeyManager(void) {
}

void KeyManager::resetKeyMappings() {
	mKeyMapping.clear();
	mCameraMapping.clear();
}

void KeyManager::resetRegisteredActions() {
	mActionMapping.clear();
	mActionTypeMapping.clear();
}

void KeyManager::writeControlsFile() {
	CIniFile writer;
	
	// write the registered actions
	std::vector<int> actions = getRegisteredActions();
	for (unsigned int i = 0; i < actions.size(); ++i) {
		const std::string name = getActionName(actions[i]);
		std::vector<OIS::KeyCode> keys = getKeys(actions[i]);
		std::string keystring;
		for (unsigned int j = 0; j < keys.size(); ++j) {
			if (j > 0) {
				keystring.append(",");
			}
			keystring.append(getKeyName(keys[j]));
		}
		std::string type = getActionType(actions[i]);
		writer.SetKeyValue(type, name, keystring);
	}

	// write the camera actions
	for (int i = CameraActions::CHANGECAMERA; i <= CameraActions::FREE_CAMERA; ++i) {
		const std::string name = getCameraName(CameraActions::CameraControllerActionType(i));
		std::vector<OIS::KeyCode> keys = getCameraKeys(CameraActions::CameraControllerActionType(i));
		std::string keystring;
		for (unsigned int j = 0; j < keys.size(); ++j) {
			if (j > 0) {
				keystring.append(",");
			}
			keystring.append(getKeyName(keys[j]));
		}
		writer.SetKeyValue("Camera", name, keystring);
	}

	// write the free roam camera actions
	for (int i = CameraActions::FREE_CAMERA_FWD; i < CameraActions::CAMERA_END; ++i) {
		const std::string name = getCameraName(CameraActions::CameraControllerActionType(i));
		std::vector<OIS::KeyCode> keys = getCameraKeys(CameraActions::CameraControllerActionType(i));
		std::string keystring;
		for (unsigned int j = 0; j < keys.size(); ++j) {
			if (j > 0) {
				keystring.append(",");
			}
			keystring.append(getKeyName(keys[j]));
		}
		writer.SetKeyValue("FreeCamera", name, keystring);
	}

	writer.Save(mControlsFile);
}

void KeyManager::registerAction(const int action, std::string name, std::string type) {
	mActionMapping[action] = name;
	mActionTypeMapping[action] = type;
}

const std::vector<OIS::KeyCode> KeyManager::getKeys(const int action) {
	std::vector<OIS::KeyCode> keys;

    std::map<OIS::KeyCode, int>::const_iterator iter;
    for (iter = mKeyMapping.begin(); iter != mKeyMapping.end(); ++iter) {
        if (iter->second == action) {
            keys.push_back(iter->first);
        }
    }

    return keys;
}

void KeyManager::setKey(const int action, const OIS::KeyCode key, bool initial) {
	mKeyMapping[key] = action;
	mCameraMapping[key] = CameraActions::INVALID;
}

void KeyManager::setKey(const int action, std::string key, bool initial) {
	setKey(action, getKeyCode(key), initial);
}

const int KeyManager::getAction(const OIS::KeyCode key) {
	if (mActive) {
		return mKeyMapping[key];
	} else {
		return 0;
	}
}

const std::vector<int> KeyManager::getRegisteredActions() {
	std::vector<int> actions;

	std::map<int, std::string>::const_iterator iter;
    for (iter = mActionMapping.begin(); iter != mActionMapping.end(); ++iter) {
        actions.push_back(iter->first);
    }

    return actions;
}

const std::string KeyManager::getActionName(const int action) {
	return mActionMapping[action];
}

const std::string KeyManager::getActionType(const int action) {
	return mActionTypeMapping[action];
}

const std::vector<OIS::KeyCode> KeyManager::getCameraKeys(const CameraActions::CameraControllerActionType action) {
	std::vector<OIS::KeyCode> keys;

    std::map<OIS::KeyCode, CameraActions::CameraControllerActionType>::const_iterator iter;
    for (iter = mCameraMapping.begin(); iter != mCameraMapping.end(); ++iter) {
        if (iter->second == action) {
            keys.push_back(iter->first);
        }
    }

    return keys;
}

void KeyManager::setCameraKey(const CameraActions::CameraControllerActionType action, const OIS::KeyCode key, bool initial) {
	mKeyMapping[key] = 0;
	mCameraMapping[key] = action;
}

void KeyManager::setCameraKey(const CameraActions::CameraControllerActionType action, std::string key, bool initial) {
	setCameraKey(action, getKeyCode(key), initial);
}

const CameraActions::CameraControllerActionType KeyManager::getCameraAction(const OIS::KeyCode key) {
	if (mActive) {
		return mCameraMapping[key];
	} else {
		return CameraActions::INVALID;
	}
}

const std::string KeyManager::getCameraName(const CameraActions::CameraControllerActionType action) {
	return CameraActions::cameraActionNames[action];
}

OIS::KeyCode KeyManager::getKeyCode(std::string name) {
	return mKeyCodeMapping[name];
}

std::string KeyManager::getKeyName(OIS::KeyCode key) {
	std::map<std::string, OIS::KeyCode>::const_iterator iter;
    for (iter = mKeyCodeMapping.begin(); iter != mKeyCodeMapping.end(); ++iter) {
        if (iter->second == key) {
            return iter->first;
        }
    }

	return "KC_UNASSIGNED";
}

void KeyManager::registerKeyCodes() {
	mKeyCodeMapping["KC_UNASSIGNED"]= OIS::KC_UNASSIGNED;
	mKeyCodeMapping["KC_ESCAPE"]= OIS::KC_ESCAPE;
	mKeyCodeMapping["KC_1"]= OIS::KC_1;
	mKeyCodeMapping["KC_2"]= OIS::KC_2;
	mKeyCodeMapping["KC_3"]= OIS::KC_3;
	mKeyCodeMapping["KC_4"]= OIS::KC_4;
	mKeyCodeMapping["KC_5"]= OIS::KC_5;
	mKeyCodeMapping["KC_6"]= OIS::KC_6;
	mKeyCodeMapping["KC_7"]= OIS::KC_7;
	mKeyCodeMapping["KC_8"]= OIS::KC_8;
	mKeyCodeMapping["KC_9"]= OIS::KC_9;
	mKeyCodeMapping["KC_0"]= OIS::KC_0;
	mKeyCodeMapping["KC_MINUS"]= OIS::KC_MINUS;
	mKeyCodeMapping["KC_EQUALS"]= OIS::KC_EQUALS;
	mKeyCodeMapping["KC_BACK"]= OIS::KC_BACK;
	mKeyCodeMapping["KC_TAB"]= OIS::KC_TAB;
	mKeyCodeMapping["KC_Q"]= OIS::KC_Q;
	mKeyCodeMapping["KC_W"]= OIS::KC_W;
	mKeyCodeMapping["KC_E"]= OIS::KC_E;
	mKeyCodeMapping["KC_R"]= OIS::KC_R;
	mKeyCodeMapping["KC_T"]= OIS::KC_T;
	mKeyCodeMapping["KC_Y"]= OIS::KC_Y;
	mKeyCodeMapping["KC_U"]= OIS::KC_U;
	mKeyCodeMapping["KC_I"]= OIS::KC_I;
	mKeyCodeMapping["KC_O"]= OIS::KC_O;
	mKeyCodeMapping["KC_P"]= OIS::KC_P;
	mKeyCodeMapping["KC_LBRACKET"]= OIS::KC_LBRACKET;
	mKeyCodeMapping["KC_RBRACKET"]= OIS::KC_RBRACKET;
	mKeyCodeMapping["KC_RETURN"]= OIS::KC_RETURN;
	mKeyCodeMapping["KC_LCONTROL"]= OIS::KC_LCONTROL;
	mKeyCodeMapping["KC_A"]= OIS::KC_A;
	mKeyCodeMapping["KC_S"]= OIS::KC_S;
	mKeyCodeMapping["KC_D"]= OIS::KC_D;
	mKeyCodeMapping["KC_F"]= OIS::KC_F;
	mKeyCodeMapping["KC_G"]= OIS::KC_G;
	mKeyCodeMapping["KC_H"]= OIS::KC_H;
	mKeyCodeMapping["KC_J"]= OIS::KC_J;
	mKeyCodeMapping["KC_K"]= OIS::KC_K;
	mKeyCodeMapping["KC_L"]= OIS::KC_L;
	mKeyCodeMapping["KC_SEMICOLON"]= OIS::KC_SEMICOLON;
	mKeyCodeMapping["KC_APOSTROPHE"]= OIS::KC_APOSTROPHE;
	mKeyCodeMapping["KC_GRAVE"]= OIS::KC_GRAVE;
	mKeyCodeMapping["KC_LSHIFT"]= OIS::KC_LSHIFT;
	mKeyCodeMapping["KC_BACKSLASH"]= OIS::KC_BACKSLASH;
	mKeyCodeMapping["KC_Z"]= OIS::KC_Z;
	mKeyCodeMapping["KC_X"]= OIS::KC_X;
	mKeyCodeMapping["KC_C"]= OIS::KC_C;
	mKeyCodeMapping["KC_V"]= OIS::KC_V;
	mKeyCodeMapping["KC_B"]= OIS::KC_B;
	mKeyCodeMapping["KC_N"]= OIS::KC_N;
	mKeyCodeMapping["KC_M"]= OIS::KC_M;
	mKeyCodeMapping["KC_COMMA"]= OIS::KC_COMMA;
	mKeyCodeMapping["KC_PERIOD"]= OIS::KC_PERIOD;
	mKeyCodeMapping["KC_SLASH"]= OIS::KC_SLASH;
	mKeyCodeMapping["KC_RSHIFT"]= OIS::KC_RSHIFT;
	mKeyCodeMapping["KC_MULTIPLY"]= OIS::KC_MULTIPLY;
	mKeyCodeMapping["KC_LMENU"]= OIS::KC_LMENU;
	mKeyCodeMapping["KC_SPACE"]= OIS::KC_SPACE;
	mKeyCodeMapping["KC_CAPITAL"]= OIS::KC_CAPITAL;
	mKeyCodeMapping["KC_F1"]= OIS::KC_F1;
	mKeyCodeMapping["KC_F2"]= OIS::KC_F2;
	mKeyCodeMapping["KC_F3"]= OIS::KC_F3;
	mKeyCodeMapping["KC_F4"]= OIS::KC_F4;
	mKeyCodeMapping["KC_F5"]= OIS::KC_F5;
	mKeyCodeMapping["KC_F6"]= OIS::KC_F6;
	mKeyCodeMapping["KC_F7"]= OIS::KC_F7;
	mKeyCodeMapping["KC_F8"]= OIS::KC_F8;
	mKeyCodeMapping["KC_F9"]= OIS::KC_F9;
	mKeyCodeMapping["KC_F10"]= OIS::KC_F10;
	mKeyCodeMapping["KC_NUMLOCK"]= OIS::KC_NUMLOCK;
	mKeyCodeMapping["KC_SCROLL"]= OIS::KC_SCROLL;
	mKeyCodeMapping["KC_NUMPAD7"]= OIS::KC_NUMPAD7;
	mKeyCodeMapping["KC_NUMPAD8"]= OIS::KC_NUMPAD8;
	mKeyCodeMapping["KC_NUMPAD9"]= OIS::KC_NUMPAD9;
	mKeyCodeMapping["KC_SUBTRACT"]= OIS::KC_SUBTRACT;
	mKeyCodeMapping["KC_NUMPAD4"]= OIS::KC_NUMPAD4;
	mKeyCodeMapping["KC_NUMPAD5"]= OIS::KC_NUMPAD5;
	mKeyCodeMapping["KC_NUMPAD6"]= OIS::KC_NUMPAD6;
	mKeyCodeMapping["KC_ADD"]= OIS::KC_ADD;
	mKeyCodeMapping["KC_NUMPAD1"]= OIS::KC_NUMPAD1;
	mKeyCodeMapping["KC_NUMPAD2"]= OIS::KC_NUMPAD2;
	mKeyCodeMapping["KC_NUMPAD3"]= OIS::KC_NUMPAD3;
	mKeyCodeMapping["KC_NUMPAD0"]= OIS::KC_NUMPAD0;
	mKeyCodeMapping["KC_DECIMAL"]= OIS::KC_DECIMAL;
	mKeyCodeMapping["KC_OEM_102"]= OIS::KC_OEM_102;	// < > | on UK/Germany keyboards
	mKeyCodeMapping["KC_F11"]= OIS::KC_F11;
	mKeyCodeMapping["KC_F12"]= OIS::KC_F12;
	mKeyCodeMapping["KC_F13"]= OIS::KC_F13;
	mKeyCodeMapping["KC_F14"]= OIS::KC_F14;
	mKeyCodeMapping["KC_F15"]= OIS::KC_F15;
	mKeyCodeMapping["KC_KANA"]= OIS::KC_KANA;
	mKeyCodeMapping["KC_ABNT_C1"]= OIS::KC_ABNT_C1;
	mKeyCodeMapping["KC_CONVERT"]= OIS::KC_CONVERT;
	mKeyCodeMapping["KC_NOCONVERT"]= OIS::KC_NOCONVERT;
	mKeyCodeMapping["KC_YEN"]= OIS::KC_YEN;
	mKeyCodeMapping["KC_ABNT_C2"]= OIS::KC_ABNT_C2;
	mKeyCodeMapping["KC_NUMPADEQUALS"]= OIS::KC_NUMPADEQUALS;
	mKeyCodeMapping["KC_PREVTRACK"]= OIS::KC_PREVTRACK;
	mKeyCodeMapping["KC_AT"]= OIS::KC_AT;
	mKeyCodeMapping["KC_COLON"]= OIS::KC_COLON;
	mKeyCodeMapping["KC_UNDERLINE"]= OIS::KC_UNDERLINE;
	mKeyCodeMapping["KC_KANJI"]= OIS::KC_KANJI;
	mKeyCodeMapping["KC_STOP"]= OIS::KC_STOP;
	mKeyCodeMapping["KC_AX"]= OIS::KC_AX;
	mKeyCodeMapping["KC_UNLABELED"]= OIS::KC_UNLABELED;
	mKeyCodeMapping["KC_NEXTTRACK"]= OIS::KC_NEXTTRACK;
	mKeyCodeMapping["KC_NUMPADENTER"]= OIS::KC_NUMPADENTER;
	mKeyCodeMapping["KC_RCONTROL"]= OIS::KC_RCONTROL;
	mKeyCodeMapping["KC_MUTE"]= OIS::KC_MUTE;
	mKeyCodeMapping["KC_CALCULATOR"]= OIS::KC_CALCULATOR;
	mKeyCodeMapping["KC_PLAYPAUSE"]= OIS::KC_PLAYPAUSE;
	mKeyCodeMapping["KC_MEDIASTOP"]= OIS::KC_MEDIASTOP;
	mKeyCodeMapping["KC_VOLUMEDOWN"]= OIS::KC_VOLUMEDOWN;
	mKeyCodeMapping["KC_VOLUMEUP"]= OIS::KC_VOLUMEUP;
	mKeyCodeMapping["KC_WEBHOME"]= OIS::KC_WEBHOME;
	mKeyCodeMapping["KC_NUMPADCOMMA"]= OIS::KC_NUMPADCOMMA;
	mKeyCodeMapping["KC_DIVIDE"]= OIS::KC_DIVIDE;
	mKeyCodeMapping["KC_SYSRQ"]= OIS::KC_SYSRQ;
	mKeyCodeMapping["KC_RMENU"]= OIS::KC_RMENU;
	mKeyCodeMapping["KC_PAUSE"]= OIS::KC_PAUSE;
	mKeyCodeMapping["KC_HOME"]= OIS::KC_HOME;
	mKeyCodeMapping["KC_UP"]= OIS::KC_UP;
	mKeyCodeMapping["KC_PGUP"]= OIS::KC_PGUP;
	mKeyCodeMapping["KC_LEFT"]= OIS::KC_LEFT;
	mKeyCodeMapping["KC_RIGHT"]= OIS::KC_RIGHT;
	mKeyCodeMapping["KC_END"]= OIS::KC_END;
	mKeyCodeMapping["KC_DOWN"]= OIS::KC_DOWN;
	mKeyCodeMapping["KC_PGDOWN"]= OIS::KC_PGDOWN;
	mKeyCodeMapping["KC_INSERT"]= OIS::KC_INSERT;
	mKeyCodeMapping["KC_DELETE"]= OIS::KC_DELETE;
	mKeyCodeMapping["KC_LWIN"]= OIS::KC_LWIN;
	mKeyCodeMapping["KC_RWIN"]= OIS::KC_RWIN;
	mKeyCodeMapping["KC_APPS"]= OIS::KC_APPS;
	mKeyCodeMapping["KC_POWER"]= OIS::KC_POWER;
	mKeyCodeMapping["KC_SLEEP"]= OIS::KC_SLEEP;
	mKeyCodeMapping["KC_WAKE"]= OIS::KC_WAKE;
	mKeyCodeMapping["KC_WEBSEARCH"]= OIS::KC_WEBSEARCH;
	mKeyCodeMapping["KC_WEBFAVORITES"]= OIS::KC_WEBFAVORITES;
	mKeyCodeMapping["KC_WEBREFRESH"]= OIS::KC_WEBREFRESH;
	mKeyCodeMapping["KC_WEBSTOP"]= OIS::KC_WEBSTOP;
	mKeyCodeMapping["KC_WEBFORWARD"]= OIS::KC_WEBFORWARD;
	mKeyCodeMapping["KC_WEBBACK"]= OIS::KC_WEBBACK;
	mKeyCodeMapping["KC_MYCOMPUTER"]= OIS::KC_MYCOMPUTER;
	mKeyCodeMapping["KC_MAIL"]= OIS::KC_MAIL;
	mKeyCodeMapping["KC_MEDIASELECT"]= OIS::KC_MEDIASELECT;
}

void KeyManager::setControlsFile(std::string file) {
	mControlsFile = file;
}

std::string KeyManager::getControlsFile() {
	return mControlsFile;
}

void KeyManager::toggleActivity() {
	mActive = !mActive;
}

void KeyManager::setActive() {
	mActive = true;
}

void KeyManager::setInactive() {
	mActive = false;
}

bool KeyManager::isActive() {
	return mActive;
}

}