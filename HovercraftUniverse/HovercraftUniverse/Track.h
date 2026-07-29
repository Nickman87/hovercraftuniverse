#ifndef TRACK_H
#define TRACK_H

#include "OgreString.h"
#include "Exception.h"
#include <tinyxml/tinyxml.h>

namespace HovUni {

class Track {
private:

	/**
	 * Name of the track
	 */
	Ogre::String mDisplayName;

	/**
	 * Physics filename
	 */
	Ogre::String mPhysicsFileName;

	/** 
	 * Path file for bots
	 */
	Ogre::String mPathFileName;

	/**
	 * Minimum players
	 */
	Ogre::int32 mMinimumPlayers;

	/**
	 * Minimum players
	 */
	Ogre::int32 mMaximumPlayers;

public:

	/**
	 * Constructor.
	 */
	Track();

	/**
	 * Constructor
	 * @param data
	 * @throws ParseException
	 */
	void load(TiXmlElement * data) throw(ParseException);

	/**
	 * Destructor
	 */
	virtual ~Track(void);

	/**
	 * Get physics filename
	 * @return the physics filename
	 */
	inline Ogre::String getPhysicsFileName() const {
		return mPhysicsFileName;
	}

	/**
	 * Set physics filename
	 * @param physics filename
	 */
	inline void setPhysicsFileName(const Ogre::String& physicsFileName){
		mPhysicsFileName = physicsFileName;
	}

	/**
	 * Get path filename
	 * @return the path filename
	 */
	inline Ogre::String getPathFileName() const {
		return mPathFileName;
	}

	/**
	 * Set path filename
	 * @param path filename
	 */
	inline void setPathFileName(const Ogre::String& pathFileName){
		mPathFileName = pathFileName;
	}

	/**
	 * Get minimum players
	 * @return the minimum players
	 */
	inline Ogre::int32 getMinimumPlayers() const {
		return mMinimumPlayers;
	}

	/**
	 * Set minimum players
	 * @param minplayers
	 */
	inline void setMinimumPlayers(Ogre::int32 minplayers){
		mMinimumPlayers = minplayers;
	}


	/**
	 * Get maximum players
	 * @return the maximum players
	 */
	inline Ogre::int32 getMaximumPlayers() const {
		return mMaximumPlayers;
	}

	/**
	 * Set maximum players
	 * @param maximum
	 */
	inline void setMaximumPlayers(Ogre::int32 maximum){
		mMaximumPlayers = maximum;
	}

	/**
	 * Get name
	 * @return the name
	 */
	inline Ogre::String getDisplayName() const {
		return mDisplayName;
	}

	/**
	 * Set name
	 * @param name
	 */
	inline void setDisplayName(const Ogre::String& name){
		mDisplayName = name;
	}
};

}

#endif