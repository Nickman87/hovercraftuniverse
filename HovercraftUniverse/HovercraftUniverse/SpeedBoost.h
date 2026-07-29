#ifndef SPEED_BOOST_H
#define SPEED_BOOST_H

#include "HovercraftUniverseEntity.h"

#include "Exception.h"
#include <tinyxml/tinyxml.h>

namespace HovUni {

class Hovercraft;

/**
 * Boost
 * A Boost should speed up an entity that passes over it in the direction it is currently heading. 
 * @author PJ
 */
class SpeedBoost : public HovercraftUniverseEntity{
private:

	/**
	 * The Boost
	 */
	Ogre::Real mBoost;

	/**
	 * The Boost gain
	 */
	Ogre::Real mGain;

public:

	/**
	 * The category used for boosts
	 */
	static const Ogre::String CATEGORY;

	/**
	 * Constructor.
	 *
	 * @param name The name of the boost
	 * @param position
	 * @param quaternion
	 * @param processInterval The process interval
	 */
	SpeedBoost(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& ogreentity, float processInterval);

	/**
	 * Constructor.
	 * 
	 * @param announcedata
	 */
	SpeedBoost( ZCom_BitStream* announcedata );

	/**
	 * Load
	 * @param data
	 * @throws ParseException
	 */
	void load(TiXmlElement * data) throw(ParseException);

	/**
	 * Destructor
	 */
	~SpeedBoost(void);

	/**
	 * Get the BoostData
	 * @return BoostData
	 */
	Ogre::Real getBoost() const {
		return mBoost;
	}

	/**
	 * Set the BoostData
	 * @param BoostData
	 */
	void setBoost(Ogre::Real boost) {
		mBoost = boost;
	}

	/**
	 * Get the Boost direction, this vector is normalised
	 * @return BoostData
	 */
	Ogre::Vector3 getBoostDirection() const;

	/**
	 * Called when given entity enters the boost
	 *
	 * @param e
	 * @return true if the entity should receive a boost, false if it should be ignored
	 */
	bool onEnter ( Entity * e );

	/**
	 * Called when given entity leaves the boost
	 *
	 * @param e
	 */
	void onLeave( Entity * e );


	/**
	 * Callback to process this entity. This allows to do entity specific processing
	 *(e.g. intermediate actions).
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
	 * @see NetworkEntity::setReplication()
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