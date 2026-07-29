#ifndef STARTPOSITION_H
#define STARTPOSITION_H

#include "HovercraftUniverseEntity.h"

#include "Exception.h"
#include <tinyxml/tinyxml.h>

namespace HovUni {

/**
 * @author PJ
 */ 
class StartPosition : public HovercraftUniverseEntity {
private:

	/**
	 * The number of the player
	 */
	Ogre::int32 mPlayerNumber;

public:

	/**
	 * The category used for startpositions
	 */
	static const Ogre::String CATEGORY;

	/**
	 * Constructor.
	 *
	 * @param name The name of the startposition
	 * @param position
	 * @param quaternion
	 * @param processInterval The process interval
	 */
	StartPosition(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, float processInterval);

	/**
	 * Constructor.
	 * 
	 * @param announcedata
	 */
	StartPosition( ZCom_BitStream* announcedata );

	/**
	 * Constructor
	 * @param data, xml element that descripes the start position
	 * @throws ParseException
	 */
	void load(TiXmlElement * data) throw(ParseException);

	/**
	 * Destructor
	 */
	~StartPosition(void);

	/**
	 * Get the player number
	 * @return player number
	 */
	inline Ogre::int32 getPlayerNumber() const {
		return mPlayerNumber;
	}

	/**
	 * Set the player number
	 * @param player number
	 */
	inline void setPlayerNumber(Ogre::int32 playernumber) {
		mPlayerNumber = playernumber;
	}

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
