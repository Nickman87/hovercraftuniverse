#include "DedicatedServer.h"
#include "Exception.h"
#include <windows.h>
#include "Config.h"

namespace HovUni {
	DedicatedServer::DedicatedServer(const std::string& configINI) {
		mConfigFilename = configINI;
	}

	DedicatedServer::~DedicatedServer() {
		mConfig->saveFile();
		delete mConfig;
		mConfig = 0;
	}

	Config* DedicatedServer::mEngineSettings = 0;
	Config* DedicatedServer::mConfig = 0;

	Config* DedicatedServer::getEngineSettings() {
		if (!mEngineSettings) {
			mEngineSettings = new Config();
		}
		return mEngineSettings;
	}

	Config* DedicatedServer::getConfig() {
		if (!mConfig) {
			mConfig = new Config();
		}
		return mConfig;
	}

	void DedicatedServer::init() {
		parseIni();
	}

	void DedicatedServer::parseIni() {
		TCHAR dirpath[MAX_PATH]=TEXT(""); 
		TCHAR** filepath={NULL};
		GetFullPathName(mConfigFilename.c_str(), MAX_PATH, dirpath, filepath);
		std::string fullConfigPath(dirpath);

		mConfig = getConfig();
		mConfig->loadFile(fullConfigPath);
		//Get(section, name, defaultValue)
		std::string mDataPath = mConfig->getValue<std::string>("Paths", "DataPath", "data");
		DWORD  retval=0;
		TCHAR  buffer[MAX_PATH]=TEXT(""); 
		TCHAR** lppPart={NULL};
		GetFullPathName(mDataPath.c_str(),MAX_PATH,buffer,lppPart);
		std::cout << "Changing Working Dir to " << buffer << std::endl;
		BOOL success = SetCurrentDirectory(buffer);
		if (!success) {
			std::string error = "Could not set working dir (check config file DataPath var)!";
			std::cerr << error << std::endl;
			//TODO Throw Exception
		}
		//Parse Engine settings
		mEngineSettings = getEngineSettings();
		mEngineSettings->loadFile(mConfig->getValue<std::string>("Server", "EngineSettings", "engine_settings.cfg"));
	}
}