#ifndef CLIENTPREPARATIONLOADER_H
#define CLIENTPREPARATIONLOADER_H

#include "Loader.h"
#include "ClientLoader.h"
#include <vector>

namespace HovUni {

class ClientPreparationEntry;

/**
 * Prepare a loader for being able to load. Since representations require the entities to exist at the client side and since we never know when that is, 
 * we load the entity names here and check upon the creation of an entity whether all entities in that scene have been loaded and if so, the 'actual' loader
 * loads the scene.
 * 
 * @author Kristof Overdulve
 */
class ClientPreparationLoader : public Loader {
private:

	/** The scenes that are to be prepared */
	std::vector<ClientPreparationEntry> mPreparationEntries; 

	/** The loader that is being used to load the data */
	ClientLoader * mLoader;

	/** The entities that belong to the loader */
	std::vector<Ogre::String> mCurrEntitiesFound;

	/** The current filename */
	Ogre::String mCurrFilename;

public:

	/**
	 * Constructor.
	 */
	ClientPreparationLoader();

	/**
	 * Destructor.
	 */
	~ClientPreparationLoader(void);

	/**
	 * Set the race state the loader belongs to
	 *
	 * @param state the race state
	 */
	virtual void setRaceState(RaceState * state);

	/**
	 * Callback when OgreMax has finished reading.
	 *
	 * @param success if successful
	 */
	virtual void FinishedLoad(bool success);

	/** 
	 * Callback when entity is read.
	 */
	void onEntity(OgreMax::Types::EntityParameters& entityparameters, const OgreMax::Types::Attachable * parent);

	/**
	 * Adds a new loader to the preparation.
	 *
	 * @param filename the name of the scene file that should be loaded
	 */
	void registerLoader(Ogre::String filename);

	/**
	 * Notifies each preparation entry that the given entity name has been registered at the client. When an entry has found all of its entries,
	 * the loader is activated.
	 *
	 * @param entityName the name of the entity just found
	 */
	void update(Ogre::String entityName);

	/**
	 * Returns the actual loader for which this preparation is being made.
	 *
	 * @return the internal loader
	 */
	ClientLoader * getInternalLoader() {
		return mLoader;
	}

};

/**
 * Represents an entry in the preparation loader. An entry is characterized by having a client loader along with its required entities and the current entities
 * already found.
 *
 * @author Kristof Overdulve
 */
class ClientPreparationEntry {
private:

	/** The client loader that should be loaded on completion */
	ClientLoader * mLoader;

	/** The entries that should still be found */
	std::vector<Ogre::String> mEntitiesToFind;

	/** The name of the file that should be loaded */
	Ogre::String mFilename;

public:

	/**
	 * Constructor.
	 *
	 * @param loader the client loader
	 * @param entitiesToFind the entities to find
	 * @param filename the name of the file to load
	 */
	ClientPreparationEntry(ClientLoader * loader, std::vector<Ogre::String> entitiesToFind, Ogre::String filename);

	/**
	 * Indicates that the given entry was found. If the current preparation required the entry to be found, it is removed from the enities to find.
	 * If the list is then empty, we return true to indicate that the loader can be activated.
	 *
	 * @param entityName the entity that was found
	 * @return true when the loader can be activated and false if not
	 */
	bool entityFound(Ogre::String entityName);

	/**
	 * Activates the loader. This only works if all entities were found.
	 */
	void activateLoader();

};

}

#endif
