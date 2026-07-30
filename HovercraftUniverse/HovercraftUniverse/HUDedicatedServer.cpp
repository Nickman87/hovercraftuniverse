#include "HUDedicatedServer.h"
#include <OgreRoot.h>
#include <OgreConfigFile.h>
#include <OgreMaterialManager.h>
#include <windows.h>
#include <math.h>

// Modern-build addition (revival Phase B, docs/porting/phase-b-collision.md):
// the one permitted game-source change for collision-geometry reconstruction.
// Havok 6.6's .hkx packfile format cannot be parsed (vendor gone, format
// undocumented), so a new module rebuilds Bullet collision shapes from the
// .scene + .mesh render data instead -- see hu_collision/OgreCollisionProvider.h
// and compat/havok/include/havok_compat/CollisionProvider.h for the seam this
// registers against. Registered here (not in CoreEngine/Application.cpp,
// despite that being the doc's illustrative example of "one registration
// call in game startup") because this method is the single call site shared
// by BOTH the standalone dedicated server process AND single-player's
// in-process local server (MainMenu::onSingleplayer's
// `new HUDedicatedServer(...); ...->run(false);`) -- exactly where physics
// loading actually happens in both topologies.
#include <hu_collision/OgreCollisionProvider.h>


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

		// Phase B collision reconstruction (see the #include comment above):
		// register the Ogre-backed CollisionProvider exactly once per
		// process, now that Ogre resource locations exist (either just
		// added above for the standalone dedicated server, or already
		// added by Application::defineResources() for single-player's
		// in-process server sharing the client's Ogre::Root) and before any
		// track/hovercraft .hkx load can race it -- HUServer/HavokThread
		// only start below. static local: this method can run again if a
		// process hosts more than one HUDedicatedServer in sequence.
		static bool collisionProviderRegistered = false;
		if (!collisionProviderRegistered) {
			hu_collision::registerDefaultProvider();
			collisionProviderRegistered = true;
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