#include "Application.h"
#include "Exception.h"
#include <tinyxml/tinyxml.h>
#include <iostream>
#include "Config.h"
#include "OgreWindowListener.h"
#include "EntityMapping.h"
// Ogre 14 API fix (docs/porting/ogre-api-gap.md): Root::showConfigDialog()
// now requires an Ogre::ConfigDialog* (the old no-argument, built-in native
// dialog overload is gone); OgreBites (already linked by hu_coreengine)
// ships a drop-in native dialog via getNativeConfigDialog().
#include <OgreBitesConfigDialog.h>

namespace HovUni {
Ogre::SceneManager* Application::msSceneMgr = 0;
Config* Application::mConfig = 0;

// Test affordance (revival Phase B, docs/porting/phase-b-plan.md): see the
// member declarations in Application.h.
Ogre::String Application::msAutoConnectHost = "";
unsigned int Application::msAutoConnectPort = 0;
bool Application::msAutoConnect = false;

Application::Application(Ogre::String appName, Ogre::String configINI) : mAppName(appName), mConfigINI(configINI) {
	// All was initialized
}

Application::~Application() {
	mConfig->saveFile();
	delete mConfig;
	mConfig = 0;
}

Config* Application::getConfig() {
	if (!mConfig) {
		mConfig = new Config();
	}
	return mConfig;
}

void Application::init() {
	parseIni();
	createRoot();
	defineResources();
	setupRenderSystem();
	createRenderWindow();
	initializeResourceGroups();
	setupInputSystem();
}

void Application::go(const Ogre::String& host, unsigned int port, bool autoConnect) {
	// Test affordance (revival Phase B, docs/porting/phase-b-plan.md): stash
	// the --autoconnect target so MainMenuState can trigger the production
	// onConnect() path itself once the menu state is ticking, without any
	// GUI interaction. host/port used to be silently discarded here (see
	// the commented-out body of createClient() below) -- this is the first
	// thing that actually reads them.
	msAutoConnectHost = host;
	msAutoConnectPort = port;
	msAutoConnect = autoConnect;

	setupScene();
	createClient(host,port);
	createFrameListener();
	startRenderLoop();
}

void Application::parseIni() {
	TCHAR dirpath[MAX_PATH]=TEXT(""); 
    TCHAR** filepath={NULL};
	GetFullPathName(mConfigINI.c_str(), MAX_PATH, dirpath, filepath);
	std::string fullConfigPath(dirpath);

	mConfig = getConfig();
	mConfig->loadFile(fullConfigPath);
	//Get(section, name, defaultValue)
	mDataPath = mConfig->getValue<std::string>("Paths", "DataPath", "data");
	mLogPath = mConfig->getValue<std::string>("Ogre", "LogFile", "HovercraftUniverse.log");
	mOgreConfig = mConfig->getValue<std::string>("Ogre", "Resources", "resources.cfg");
	mOgrePlugins = mConfig->getValue<std::string>("Ogre", "Plugins", "plugins.cfg");
	mSoundPath = mConfig->getValue<std::string>("Sound", "Path", "sound\\");
	mSoundFile = mConfig->getValue<std::string>("Sound", "File", "Sound.fev");
	mControlsPath = mConfig->getValue<std::string>("Controls", "Path", "controls\\");
	mControlsFile = mConfig->getValue<std::string>("Controls", "File", "Controls.ini");
	mEntitiesPath = mConfig->getValue<std::string>("Entities", "Path", "entities\\");
	mEntitiesFile = mConfig->getValue<std::string>("Entities", "File", "Entities.ini");

	// Set file to entity manager 
	EntityMapping::setEntityIniFile(mEntitiesPath + mEntitiesFile);

	//WARNING! Sets the current directory to the Data Folder, relative to current PWD.
	DWORD  retval=0;
	TCHAR  buffer[MAX_PATH]=TEXT(""); 
    TCHAR** lppPart={NULL};
	GetFullPathName(mDataPath.c_str(),MAX_PATH,buffer,lppPart);
	std::cout << "Changing Working Dir to " << buffer << std::endl;
	BOOL success = SetCurrentDirectory(buffer);
	if (!success) {
		//TODO error handling
		std::cerr << "Could not set working dir (check config file DataPath var)!" << std::endl;
	}

}

void Application::createRoot() {
	mOgreRoot = new Ogre::Root(mOgrePlugins.c_str(), "ogre.cfg", mLogPath);
	std::cout << "Creating log at " << mLogPath << std::endl;
	// Ogre 14 API fix (docs/porting/ogre-api-gap.md): see the
	// OgreOverlaySystem.h include comment in Application.h -- must be
	// constructed with Root created but not yet initialised.
	mOverlaySystem = new Ogre::OverlaySystem();
}

void Application::defineResources() {
	// Load config
	Ogre::ConfigFile cf;
	cf.load("resources.cfg");

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
}

void Application::setupRenderSystem() {
	if (!mOgreRoot->restoreConfig() && !mOgreRoot->showConfigDialog(OgreBites::getNativeConfigDialog())) {
		// TODO Throw exception
	}
}

void Application::createRenderWindow() {
	mOgreRoot->initialise(true, mAppName);
}

void Application::initializeResourceGroups() {
	Ogre::TextureManager::getSingleton().setDefaultNumMipmaps(5);
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
}

void Application::createClient(const Ogre::String& host, unsigned int port){
	// Client
	/*
	mClient = new ClientCore(host.c_str(), port);

	TiXmlDocument doc("gui/GUIConfig.xml");
	doc.LoadFile();

	mGameStateMgr = new GameStateManager(mInputManager, GameStateManager::IN_GAME, new InGameState(mClient, doc.RootElement()->FirstChildElement("HUD")));
	*/

	// TODO MAIN_MENU is not generic enough (added by Kristof)
	mGameStateMgr = new GameStateManager(mInputManager, GameStateManager::MAIN_MENU, getInitialGameState());
}

void Application::setupScene() {
	// Create scene manager
	// Ogre 14 API fix (docs/porting/ogre-api-gap.md): Ogre::ST_GENERIC / the
	// SceneType-enum overload of createSceneManager no longer exists --
	// modern Ogre selects a scene manager by its (string) factory type name.
	msSceneMgr = mOgreRoot->createSceneManager("DefaultSceneManager", "Default");
	// Ogre 14 API fix (docs/porting/ogre-api-gap.md): register the Overlay
	// component's bootstrap object (created in createRoot()) as a
	// RenderQueueListener on this scene manager -- required for
	// Ogre::OverlayManager::getSingleton() to be valid at all (it did not
	// need this manual step pre-1.9) and for overlays to actually render.
	msSceneMgr->addRenderQueueListener(mOverlaySystem);
	msSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_TEXTURE_ADDITIVE_INTEGRATED);

	// Get created window
	Ogre::RenderWindow* win = mOgreRoot->getAutoCreatedWindow();
	Ogre::WindowEventUtilities::addWindowEventListener(win, new OgreWindowListener());

	// TODO Do the operations below belong here or as in separated methods
	// Set ambient light
	// msSceneMgr->setAmbientLight(Ogre::ColourValue(0.25, 0.25, 0.25));

	// Create and store entity manager
	mEntityManager = EntityManager::getClientSingletonPtr();

	// Create and store representation manager
	RepresentationManager::initialise(mEntityManager, msSceneMgr);
	mRepresentationManager = RepresentationManager::getSingletonPtr();

	//Get the GUI config file
	//Load and parse the config
	TiXmlDocument doc("gui/GUIConfig.xml");
	doc.LoadFile();

	TiXmlElement* root = doc.RootElement();

	//////////////////////////////////////////////////////////////////////////
	//TODO: The creation of a GameView should be moved to InGameState (Nick)//
	//////////////////////////////////////////////////////////////////////////
    // Add single game view to representation manager and fix aspect ratio

	GameView * gv = new GameView(msSceneMgr);
    Ogre::Camera * cam = gv->getCamera()->getCamera();
    Ogre::Viewport * vp = win->addViewport(cam);
    cam->setAspectRatio(Ogre::Real(vp->getActualWidth()) / Ogre::Real(vp->getActualHeight()));
    mRepresentationManager->addGameView(gv);

    // Initialise and store the GUIManager
    GUIManager::init(root->Attribute("mediaPath"), vp);
    mGUIManager = GUIManager::getSingletonPtr();

	// Initialise and store the SoundManager (dont remove trailing \)
	SoundManager::init(mSoundPath, mSoundFile);
	mSoundManager = SoundManager::getSingletonPtr();

	// Allow an inheriting class to play music
	playMusic(mSoundManager);

	// Allow some custom setup for inheriting classes
	customSceneSetup();
}

void Application::setupInputSystem() {
	mInputManager = InputManager::getSingletonPtr();
	mInputManager->initialise(mOgreRoot->getAutoCreatedWindow());
	mInputManager->setControlsFile(mControlsPath + mControlsFile);
}

void Application::createFrameListener() {
	//mFrameListener = new ApplicationFrameListener(mOgreRoot->getSceneManager("Default"), mEntityManager, mRepresentationManager,
	//	mInputManager, mClient);
	//mOgreRoot->addFrameListener(mFrameListener);
	mOgreRoot->addFrameListener(mGameStateMgr);
}

void Application::startRenderLoop() {
	mOgreRoot->startRendering();
}

}