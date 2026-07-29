#ifndef REPRESENTATIONMANAGER_H_
#define REPRESENTATIONMANAGER_H_

#include <vector>
#include "EntityManager.h"
#include "EntityRepresentation.h"
#include "GameView.h"

using namespace std;

namespace HovUni {

/**
 * The representation manager keeps a list of entity representations in a spatial hierarchy
 * suitable for quick visibility determination.
 *
 * @author Kristof Overdulve
 */
class RepresentationManager {
protected:

	/** The entity manager */
	EntityManager * mEntityManager;

	/** The ogre scene manager */
	Ogre::SceneManager * mSceneMgr;

	/** The list of entity representations to update */
	std::vector<EntityRepresentation *> mEntityRepresentations;

	/** The list of game views to draw */
	std::vector<GameView *> mGameViews;

	/** The singleton object */
    static RepresentationManager * mRepresentationManager;

public:

	/**
	 * Constructor.
	 *
	 * @param entityMgr the entity manager to use to check the entities
	 * @param sceneMgr the Ogre scene manager 
	 */
	RepresentationManager(EntityManager * entityMgr, Ogre::SceneManager * sceneMgr);

	/**
	 * Initialises the entity manager.
	 *
	 * @param entityMgr the entity manager to use to check the entities
	 * @param sceneMgr the Ogre scene manager 
	 */
	static void initialise(EntityManager * entityMgr, Ogre::SceneManager * sceneMgr);

	/**
	 * Destructor.
	 */
	virtual ~RepresentationManager();

	/**
	 * Adds a new entity representation to the manager.
	 *
	 * @param entityRep the entity representation to update
	 */
	void addEntityRepresentation(EntityRepresentation * entityRep);

	/**
	 * Returns the entity representations.
	 *
	 * @return the entity representations
	 */
	std::vector<EntityRepresentation *> getEntityRepresentations() { return mEntityRepresentations; }

	/**
	 * Removes the given entity representation from the manager.
	 *
	 * @param entityRep the unique name of the entity representation to remove
	 */
	void removeEntityRepresentation(Ogre::String entityRep);

	/**
	 * Adds a game view to draw in each frame.
	 *
	 * @param gameView the game view to draw in each frame
	 */
	void addGameView(GameView * gameView);

	/**
	 * Returns the game views.
	 *
	 * @return the game views
	 */
	std::vector<GameView *> getGameViews() { return mGameViews; }

	/**
	 * Returns the entity manager singleton.
	 *
	 * @return the singleton pointer
	 */
    static RepresentationManager * getSingletonPtr(void);

	/**
	 * Returns the Ogre scene manager.
	 *
	 * @return the scene manager
	 */
	Ogre::SceneManager * getSceneManager() { return mSceneMgr; }

	/**
	 * Draw all the game views.
	 *
	 * @param timeSinceLastFrame the time that elapsed since the last frame
	 */
	void drawGameViews(Ogre::Real timeSinceLastFrame);

	/**
	 * Request the representation of the tracked entity
	 *
	 * @return The representation of the tracked entity, 0 when it was not found
	 */
	EntityRepresentation* getTrackedEntityRepresentation();

protected:

	/**
	 * Updates the representations at frame rate.
	 */
	void updateRepresentations();

};

}

#endif