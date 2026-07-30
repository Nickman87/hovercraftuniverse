#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <OgreRoot.h>
// Ogre 14 API fix (docs/porting/ogre-api-gap.md): Overlay was split out of
// core Ogre into a separate component; Root no longer auto-creates the
// OverlayManager/FontManager, an explicit Ogre::OverlaySystem must be
// constructed and registered as a RenderQueueListener with each
// SceneManager -- see createRoot()/setupScene() in Application.cpp. Without
// this, Ogre::OverlayManager::getSingletonPtr() (called by
// MouseVisualisation's ctor and every GUI overlay) returns null and the
// first overlay created access-violates.
#include <OgreOverlaySystem.h>
// Ogre 14 API fix (docs/porting/ogre-api-gap.md): see the
// DuplicateMaterialScriptCompilerListener.h comment -- installs a
// ScriptCompilerListener that restores Ogre 1.7's first-definition-wins
// handling of the shipped .material scripts' duplicate material names.
#include "DuplicateMaterialScriptCompilerListener.h"
#include "EntityManager.h"
#include "InputManager.h"
#include "GameStateManager.h"
#include "RepresentationManager.h"
#include <GUIManager.h>
#include <SoundManager.h>
#include <string>
#include "Config.h"

namespace HovUni {

/**
 * Main application for a game using the core engine.
 *
 * @author Kristof Overdulve, Dirk Delahaye
 */
class Application {
protected:

	/** The name of the application */
	Ogre::String mAppName;

	/**
	*	The singleton Config object.
	*/
	static Config* mConfig;

	/** The config INI filename */
	Ogre::String mConfigINI;

	/** The root Ogre object */
	Ogre::Root * mOgreRoot;

	/** The Overlay component's bootstrap object (see the OgreOverlaySystem.h
	 * include comment above) -- created in createRoot(), registered with
	 * the scene manager in setupScene(). */
	Ogre::OverlaySystem * mOverlaySystem;

	/** The game state manager */
	GameStateManager* mGameStateMgr;

	/** The entity manager */
	EntityManager * mEntityManager;

	/** The input manager */
	InputManager * mInputManager;

	/** The representation manager */
	RepresentationManager * mRepresentationManager;

	/** The GUI Manager */
	GUIManager * mGUIManager;

	/** The sound manager */
	SoundManager * mSoundManager;

	// INI file values
	/** The path to the data folder */
	std::string mDataPath;
	/** The path to the log file */
	std::string mLogPath;
	/** The path to the ogre config file */
	std::string mOgreConfig;
	/** The path to the ogre plgins file */
	std::string mOgrePlugins;
	/** The path to the sound directory */
	std::string mSoundPath;
	/** The path to the sound file, relative from the sound directory */
	std::string mSoundFile;
	/** The path to the controls directory */
	std::string mControlsPath;
	/** The path to the controls file */
	std::string mControlsFile;
	/** The path to the entities path */
	std::string mEntitiesPath;
	/** The path to the entities file */
	std::string mEntitiesFile;

	// Test affordance (revival Phase B, docs/porting/phase-b-plan.md): go()
	// used to receive host/port and silently discard them (see the
	// commented-out body of createClient() below). A two-process test
	// harness needs a way to auto-connect a client to an already-running
	// dedicated server with no GUI interaction, so go() now stashes its
	// arguments here; MainMenuState reads them back on its first
	// frameStarted() tick to (optionally) drive the real onConnect() path
	// itself. Static, like mConfig above, since Application has no
	// singleton accessor but game states outside this class need to read
	// them. Not original behaviour -- with no --autoconnect flag,
	// msAutoConnect stays false and nothing changes.
	static Ogre::String msAutoConnectHost;
	static unsigned int msAutoConnectPort;
	static bool msAutoConnect;

	// Test affordance (revival Phase B, docs/porting/phase-b-plan.md):
	// --autostart implies --autoconnect and additionally has LobbyState fire
	// mLobby->start() itself once the client is recognised as admin, so a
	// test harness can drive a race past the lobby with no GUI interaction.
	// Not original behaviour -- with no --autostart flag, msAutoStart stays
	// false and nothing changes.
	static bool msAutoStart;

public:

	/**
	*	Returns a pointer to the singleton Config object.
	*	@return	The Config object (singleton).
	*/
	static Config* getConfig();

	/** The scene manager */
	static Ogre::SceneManager * msSceneMgr;

	/**
	 * Constructor.
	 *
	 * @param appName the name of the application/game
	 * @param configINI the INI file that contains all initialization and configuration properties in order to run the application properly
	 */
	Application(Ogre::String appName, Ogre::String configINI);

	/**
	 * Destructor.
	 */
	~Application();

	/**
	 * Initialize all things
	 */
	void init();

	/**
	 * The main method that triggers the application to run.
	 *
	 * @param host the hostname to connect to
	 * @param port the port to connect on
	 * @param autoConnect test affordance (revival Phase B, docs/porting/phase-b-plan.md):
	 *        when true, the menu state connects to host:port itself on its
	 *        first tick instead of waiting for a GUI click. Defaults to
	 *        false, i.e. unchanged original behaviour.
	 * @param autoStart test affordance (revival Phase B, docs/porting/phase-b-plan.md):
	 *        when true (implies autoConnect), LobbyState fires
	 *        mLobby->start() itself once we're recognised as admin in the
	 *        lobby. Defaults to false, i.e. unchanged original behaviour.
	 */
	void go(const Ogre::String& host, unsigned int port, bool autoConnect = false, bool autoStart = false);

	/**
	 * Test affordance (revival Phase B, docs/porting/phase-b-plan.md): read
	 * back the host/port/autoConnect/autoStart that were passed to go(), so a
	 * game state can drive an auto-connect/auto-start without any GUI
	 * interaction.
	 */
	static const Ogre::String& getAutoConnectHost() { return msAutoConnectHost; }
	static unsigned int getAutoConnectPort() { return msAutoConnectPort; }
	static bool getAutoConnect() { return msAutoConnect; }
	static bool getAutoStart() { return msAutoStart; }

	/**
	 * Creates the client.
	 * 
	 * @param host the hostname to connect to
	 * @param port the port to connect on
	 */
	void createClient(const Ogre::String& host, unsigned int port);

	/**
	 * Allows the inheriting class to define an initial game state.
	 *
	 * @return the initial game state
	 */
	virtual BasicGameState * getInitialGameState() = 0;

	/**
	 * Parse the INI file.
	 */
	void parseIni();

	/**
	 * Creates the root object.
	 */
	void createRoot();

	/**
	 * Defines the used resources.
	 */
	void defineResources();

	/**
	 * Sets up the render system.
	 */
	void setupRenderSystem();

	/**
	 * Creates the window that is rendered.
	 */
	void createRenderWindow();

	/**
	 * Initializes the resource groups.
	 */
	void initializeResourceGroups();

	/**
	 * Sets up the scene that should be displayed.
	 */
	void setupScene();

	/**
	 * Implement this method in order to play music. Leave empty if no music must be played.
	 *
	 * @param soundMgr the sound manager to use to play the music
	 */
	virtual void playMusic(SoundManager * soundMgr) = 0;

	/**
	 * Implement this method in order to perform custom scene operations.
	 */
	virtual void customSceneSetup() = 0;

	/**
	 * Creates input managers in order to be able to process input later.
	 */
	void setupInputSystem();

	/**
	 * Creates the frame listener.
	 */
	void createFrameListener();

	/**
	 * Starts rendering.
	 */
	void startRenderLoop();

};

}

#endif
