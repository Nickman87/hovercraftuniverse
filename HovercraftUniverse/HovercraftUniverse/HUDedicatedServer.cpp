#include "HUDedicatedServer.h"
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreMaterialManager.h>
#include <windows.h>
#include <math.h>


namespace HovUni {
	HUDedicatedServer::HUDedicatedServer(const std::string& configINI) : 
		DedicatedServer(configINI), mServer(0) {
	}

	HUDedicatedServer::~HUDedicatedServer() {
		if (mServer) {
			mServer->stop();
		}
		delete mServer;
		mServer = 0;
	}

	void HUDedicatedServer::run(bool standalone) {
		Ogre::Root* ogreRoot = Ogre::Root::getSingletonPtr();
		
		if (ogreRoot == 0) {
			ogreRoot = new Ogre::Root(getConfig()->getValue<std::string>("Ogre", "Plugins", "plugins.cfg"), getConfig()->getValue<std::string>("Ogre", "ConfigFile", "ogre.cfg"), getConfig()->getValue<std::string>("Ogre", "LogFile", "Server.log"));
			Ogre::ConfigFile cf;
			cf.load(mConfig->getValue<std::string>("Ogre", "Resources", "resources.cfg").c_str());
			// Iterate over config
			Ogre::ConfigFile::SectionIterator seci = cf.getSectionIterator();
			while (seci.hasMoreElements()) {
				// Read property
				Ogre::String secName = seci.peekNextKey();
				Ogre::ConfigFile::SettingsMultiMap * settings = seci.getNext();
				
				// For all settings of that property, add them
				for (Ogre::ConfigFile::SettingsMultiMap::iterator it = settings->begin(); it != settings->end(); it++) {
					Ogre::String typeName = it->first;
					Ogre::String archName = it->second;
					Ogre::ResourceGroupManager::getSingleton().addResourceLocation(archName, typeName, secName);
				}
			}
			
			//make sure it doesn't parse materials
			Ogre::ResourceGroupManager::getSingleton()._unregisterScriptLoader(Ogre::MaterialManager::getSingletonPtr());
		}

		//Save the INI here to make it complete
		getConfig()->saveFile();

		mServer = new HUServer();
		mServer->start();

		if (standalone) {
			mServer->join();
			delete mServer;
			mServer = 0;
			delete ogreRoot;
			ogreRoot = 0;
		}
	}

	void HUDedicatedServer::init() {
		DedicatedServer::init();
	}

	void HUDedicatedServer::stop() {
		if (mServer != 0) {
			if (mServer->isRunning()) {
				mServer->stop();
			}
		}
	}
}