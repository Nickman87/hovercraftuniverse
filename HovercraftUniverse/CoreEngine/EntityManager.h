#ifndef ENTITYMANAGER_H_
#define ENTITYMANAGER_H_

#include <vector>
#include "Entity.h"

namespace HovUni {

/**
 * The entity manager contains all entities. They must be added to the entity manager, so that
 * the entity manager can update them at fixed frequencies or can search for them when queried.
 *
 * @author Kristof Overdulve
 */
class EntityManager {
protected:

	/** The entities that have been registered at the manager */
	std::vector<Entity *> mEntities;

	/** The new entities since last call */
	std::vector<Entity *> mNewEntities;

	/** The removed entities since last call */
	std::vector<Entity *> mRemovedEntities;

	/** The entity that is being tracked by the camera */
	Ogre::String mEntityTracked;

	/** The singleton object for the server */
    static EntityManager * mEntityManagerServer;

	/** The singleton object for the client */
    static EntityManager * mEntityManagerClient;

public:

	/**
	 * Category that captures all entities
	 */
	static const std::string ALL;
	
	/**
	 * Constructor.
	 */
	EntityManager();

	/**
	 * Destructor.
	 */
	virtual ~EntityManager();

	/**
	 * Register an entity with the entity manager.
	 *
	 * @param entity the entity that ought to be registered with the manager
	 */
	void registerEntity(Entity * entity);

	/**
	 * Indicates to track the entity with the given name. We do not allow the entity object to 
	 * be passed directly since this will be called in the constructor before the entity is fully
	 * created. Only one entity can be track at a time.
	 *
	 * @param entityName the name of the entity to track
	 */
	void trackEntity(Ogre::String entityName);

	/**
	 * Releases an entity with the given name from the entity manager.
	 *
	 * @param entityName the unique name of the entity to remove
	 */
	void releaseEntity(Ogre::String entityName);

	/**
	 * Returns the entity with the given name from the entity manager or null. 
	 *
	 * @param entityName the unique name of the entity to return
	 * @return the entity with the given name
	 */
	Entity * getEntity(Ogre::String entityName);

	/**
	 * Returns the entities with the given category from the entity manager or an empty list.
	 *
	 * @param categoryName the name of the category to return the entities from
	 * @return the entities belonging to the given category
	 */
	std::vector<Entity *> getEntities(Ogre::String categoryName);

	/**
	 * Returns all the entities in the manager
	 *
	 * @return the entities
	 */
	std::vector<Entity *> getAllEntities();

	/** 
	 * Returns the entity that is to be tracked.
	 *
	 * @return the entity that is tracked
	 */
	Entity * getTrackedEntity();

	/**
	 * Indicates whether the entity manager has new entities.
	 *
	 * @return true when the entity manager has new entities and false if not
	 */
	bool hasNewEntities() { return (mNewEntities.size() > 0); }

	/**
	 * Indicates whether the entity manager has removed entities.
	 *
	 * @return true when the entity manager has removed entities and false if not
	 */
	bool hasRemovedEntities() { return (mRemovedEntities.size() > 0); }

	/**
	 * Returns the entities since last call and clears the list.
	 * 
	 * @return the new entities
	 */
	std::vector<Entity *> getNewEntities();

	/**
	 * Returns the removed entities since last call and clears the list.
	 *
	 * @return the removed entities
	 */
	std::vector<Entity *> getRemovedEntities();

	/**
	 * Updates the entities.
	 *
	 * @param timeSinceLastFrame the time that elapsed since the last frame
	 */
	void updateEntities(Ogre::Real timeSinceLastFrame);

	/**
	 * Returns the entity manager singleton for the server.
	 *
	 * @return the singleton pointer
	 */
    static EntityManager * getServerSingletonPtr(void);

	/**
	 * Returns the entity manager singleton for the client.
	 *
	 * @return the singleton pointer
	 */
    static EntityManager * getClientSingletonPtr(void);

};

}

#endif /* ENTITYMANAGER_H_ */