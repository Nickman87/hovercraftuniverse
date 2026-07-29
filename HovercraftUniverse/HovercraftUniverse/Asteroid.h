#ifndef ASTEROID_H
#define ASTEROID_H

#include "HovercraftUniverseEntity.h"
#include <tinyxml/tinyxml.h>
#include "Exception.h"

namespace HovUni {

/**
 * The asteroid is an entity on which the characters will race and on which the track will be placed. Every asteroid has a gravitational field
 * that exercises forces on the entities that are placed onto it.
 *
 * @author PJ
 */
class Asteroid : public HovercraftUniverseEntity {
public:

	/**
	 * The type of asteroid on which the race will take place.
	 */
	enum AsteroidType {
		UNKNOWN = -1,
		ICE,
		FIRE,
		JUNGLE,
		STONE,
		DESERT
	};

private:

	/** The name of the asteroid */ 
	Ogre::String mDisplayName;

	/** The force of the gravitational field of the asteroid */
	Ogre::Real mGravity;

	/** The type of asteroid */
	AsteroidType mAsteroidType;

public:

	/**
	 * The category used for asteroids
	 */
	static const Ogre::String CATEGORY;
	
	/**
	 * Constructor.
	 *
	 * @param name The name of the asteroid
	 * @param position
	 * @param quaternion
	 * @param processInterval The process interval
	 */
	Asteroid(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& ogreentity, float processInterval);

	/**
	 * Constructor.
	 * 
	 * @param announcedata
	 */
	Asteroid( ZCom_BitStream* announcedata );

	/**
	 * Load
	 * @param data, xml element that descripes the asteroid
	 * @throws ParseException
	 */
	void load(TiXmlElement * data) throw(ParseException);

	/**
	 * Destructor
	 */
	~Asteroid(void);

	/**
	 * Get the name.
	 *
	 * @return name
	 */
	inline Ogre::String getDisplayName() const {
		return mDisplayName;
	}

	/**
	 * Set the name.
	 *
	 * @param name the name
	 */
	inline void setDisplayName( const Ogre::String& name ) {
		mDisplayName = name;
	}

	/**
	 * Get the gravity.
	 *
	 * @return the gravity
	 */
	inline Ogre::Real getGravity() const {
		return mGravity;
	}

	/**
	 * Set the gravity.
	 *
	 * @param gravity the gravity
	 */
	inline void setGravity( const Ogre::Real& gravity ) {
		mGravity = gravity;
	}

	/**
	 * Get the AsteroidData type.
	 *
	 * @return type the type
	 */
	inline AsteroidType getAsteroidType() const {
		return mAsteroidType;
	}

	/**
	 * Set the AsteroidData type.
	 *
	 * @param type the type
	 */
	inline void setAsteroidType( AsteroidType type ) {
		mAsteroidType = type;
	}

	/**
	 * Callback to process this entity. This allows to do entity specific processing
	 * (e.g. intermediate actions).
	 *
	 * @param timeSince the time since the last processing of the entity
	 */
	virtual void process(float timeSince){
	}

	/**
	 * Callback to process a controller event at the server that got processed by the 
	 * controller.  Must be overriden since this class in itself has no clue which 
	 * controller properties there are.
	 *
	 * @param event a controller event
	 */
	virtual void processEventsServer(ControllerEvent* cEvent){
	}

	/**
	 * Callback to process a controller event at the owner that got processed by the 
	 * controller.  Must be overriden since this class in itself has no clue which 
	 * controller properties there are.
	 *
	 * @param event a controller event
	 */
	virtual void processEventsOwner(ControllerEvent* cEvent){
	}

	/**
	 * Callback to process a controller event at other clients that got processed by the controller.  Must
	 * be overriden since this class in itself has no clue which controller properties 
	 * there are.
	 *
	 * @param event a controller event
	 */
	virtual void processEventsOther(ControllerEvent* cEvent){
	}

	/**
	 * @see NetworkEntity
	 */
	void setupReplication();

	/**
	 * Get the class name for this class. This is used for registering
	 * the class with the network
	 *
	 * @return the class name
	 */
	static std::string getClassName();
};

}

#endif