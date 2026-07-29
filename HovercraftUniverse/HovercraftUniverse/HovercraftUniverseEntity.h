#ifndef HOVERCRAFTUNIVERSEENTITY_H
#define HOVERCRAFTUNIVERSEENTITY_H

#include "Entity.h"

namespace HovUni {

class RaceState;

/**
 * Base class for all Hovercraft Universe Entities
 */
class HovercraftUniverseEntity: public Entity {
protected:

	/** The racestate */
	RaceState * mRacestate;

public:

	/**
	 * Constructor.
	 *
	 * @param name the unique name of the entity
	 * @param category the category to which this entity belongs
	 * @param position the initial position of the entity
	 * @param orientation the initial orientation of the entity
	 * @param upvector the up vector
	 * @param name of the ogre entity that represents this entity
	 * @param processInterval the mean interval between two consecutive processings (-1 for no process callbacks)
	 * @param replicators the number of replicator to be used
	 */
	HovercraftUniverseEntity(const Ogre::String& name, const Ogre::String& category, const Ogre::Vector3& position,
			const Ogre::Vector3& orientation, const Ogre::Vector3& upvector, const Ogre::String& mOgreEntity,
			float processInterval, unsigned short replicators);

	/**
	 * Constructor.
	 *
	 * @param name the unique name of the entity
	 * @param category the category to which this entity belongs
	 * @param position the initial position of the entity
	 * @param orientation the initial orientation of the entity
	 * @param name of the ogre entity that represents this entity
	 * @param processInterval the mean interval between two consecutive processings (-1 for no process callbacks)
	 * @param replicators the number of replicator to be used
	 */
	HovercraftUniverseEntity(const Ogre::String& name, const Ogre::String& category, const Ogre::Vector3& position,
			const Ogre::Quaternion& orientation, const Ogre::String& mOgreEntity, float processInterval,
			unsigned short replicators);

	/**
	 * Constructor.
	 *
	 * @param announcementdata the data send by the server
	 * @param category the category to which this entity belongs
	 * @param replicators the number of replicator to be used
	 */
	HovercraftUniverseEntity(ZCom_BitStream* announcementdata, const Ogre::String& category, unsigned short replicators);

	/**
	 * Destructor
	 */
	virtual ~HovercraftUniverseEntity(void);

	/**
	 * Set the race state for the entity
	 *
	 * @param racestate the race state
	 */
	virtual void setRaceState(RaceState* racestate);

	/**
	 * Get the race state for the entity
	 *
	 * @return the race state
	 */
	RaceState* getRaceState();
};

}

#endif
