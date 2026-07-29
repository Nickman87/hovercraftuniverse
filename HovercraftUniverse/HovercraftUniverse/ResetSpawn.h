#ifndef RESETSPAWN_H
#define RESETSPAWN_H

#include "HovercraftUniverseEntity.h"
#include <tinyxml/tinyxml.h>
#include "Exception.h"

namespace HovUni {

/**
 * @author PJ
 */ 
class ResetSpawn : public HovercraftUniverseEntity {
private:

	//The reset id this checkpooint is connected too
	int mReset;

public:	

	/**
	 * The category used for reset spawns
	 */
	static const Ogre::String CATEGORY;

	/**
	 * Constructor.
	 *
	 * @param name The name of the hovercraft
	 * @param position
	 * @param quaternion
	 * @param processInterval The process interval
	 */
	ResetSpawn(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, float processInterval);

	/**
	 * Constructor.
	 * 
	 * @param announcedata
	 */
	ResetSpawn( ZCom_BitStream* announcedata );

	/**
	 * Destructor
	 */
	~ResetSpawn(void);

	/**
	 * Get the id of the checkpoint the reset is coupled too
	 * @param checkpoint
	 */
	inline int getCheckpointId(){
		return mReset;
	}

	/**
	 * load
	 * @param data, xml element that descripes the reset spawn
	 * @throws ParseException
	 */
	void load(TiXmlElement * data) throw(ParseException);


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
	 * Get the class name for this class. This is used for registering
	 * the class with the network
	 *
	 * @return the class name
	 */
	static std::string getClassName();
};

}

#endif
