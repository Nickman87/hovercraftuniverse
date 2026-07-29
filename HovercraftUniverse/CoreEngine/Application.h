#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <OgreRoot.h>
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
	 */
	void go(const Ogre::String& host, unsigned int port);

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
