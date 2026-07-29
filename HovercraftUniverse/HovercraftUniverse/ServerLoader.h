#ifndef SERVERLOADER_H
#define SERVERLOADER_H

#include "Loader.h"
#include "GameEntityDeclarations.h"

namespace HovUni {

class HoverCraftUniverseWorld;

/**
 * Loader for the server
 * @author Pieter-Jan Pintens
 */
class ServerLoader: public Loader {
private:

	/**
	 * Havok world
	 */
	HoverCraftUniverseWorld * mHovercraftWorld;

	/**
	 * A flag to state that the loader is working on hovercrafts
	 */
	bool mLoadingHovercrafts;

	/**
	 * The path for bots
	 */
	Ogre::String mPathFilename;

	/**
	 * Current player
	 */
	RacePlayer* mPlayer;
	Ogre::String mCurrentHovercraft;

	/**
	 * Current position
	 */
	int mPosition;

public:

	ServerLoader();

	~ServerLoader();

	/**
	 * Load given track
	 * @param filename
	 */
	virtual void load(const Ogre::String& filename);	

	//Callbacks that need implementations

	/**
	 * Callback when track loading done, after this step, the hovercrafts will be loaded
	 */
	virtual void FinishedLoad(bool success);

	/**
	 * Needed for Track
	 */
	virtual void onSceneUserData(const Ogre::String& userDataReference, const Ogre::String& userData);

	/**
	 * Needed for all other game items
	 */
	virtual void onExternal(OgreMax::Types::ExternalItem& externalitem);

	/**
	 * Needed to read hovercraft and other entities that are loaded later
	 */
	virtual void onEntity(OgreMax::Types::EntityParameters& entityparameters, const OgreMax::Types::Attachable * parent);

private:

	/**
	 * Parse the given root for game entity properties
	 * @param root, the root must not be 0
	 * @param ogreentity, will be filled
	 * @param processtime, will be filled
	 */
	void parseEntityParameters(TiXmlElement * root, Ogre::String& ogreentity, Ogre::Real& processtime);

	/**
	 * Read the metadata from the external item, create a gameenity from this.
	 * Throw ParseException if exteralitem contains invallid information.
	 * @param mExternalitem
	 */
	void parseWorldUserData(OgreMax::Types::ExternalItem& mExternalitem);

	/**
	 * This will read the track metadata, this metadata contains the filename of the physical representation.
	 * This is loaded into havok. Throws a ParseException if something fails.
	 * @param data
	 */
	void parseTrackUserData(const Ogre::String& data);

	/**
	 * This will read the metadata of the given entity which should be an hovercraft.
	 * Will load the hovercraft and add it to the world. Throws a ParseException if something fails.
	 * @param entityparameters
	 */
	void parseHovercraftUserData(OgreMax::Types::EntityParameters& entityparameters);
};

}

#endif
