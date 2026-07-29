#ifndef START_H
#define START_H

#include "HovercraftUniverseEntity.h"

#include "Exception.h"
#include <tinyxml/tinyxml.h>

namespace HovUni {

class Hovercraft;

/**
 * Start 
 * A location in the world on where hovercrafts will be placed when the race begins.
 * @author PJ
 */ 
class Start : public HovercraftUniverseEntity {
public:

	/**
	 * The category used for starts
	 */
	static const Ogre::String CATEGORY;

	/**
	 * Constructor.
	 *
	 * @param name The name of the start
	 * @param position
	 * @param quaternion
	 * @param processInterval The process interval
	 */
	Start(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& ogreentity, float processInterval);

	/**
	 * Constructor.
	 * 
	 * @param announcedata
	 */
	Start( ZCom_BitStream* announcedata );

	/**
	 * load 
	 * @param data, xml element that descripes the start
	 * @throws ParseException
	 */
	void load(TiXmlElement * data) throw(ParseException);

	/**
	 * Destructor
	 */
	~Start(void);

	/**
	 * Called when given hovercraft enters the start bounding box
	 * @param hovercraft
	 */
	void onEnter ( Hovercraft * hovercraft );

	/**
	 * Called when given hovercraft leaves the start bounding box
	 * @param hovercraft
	 */
	void onLeave( Hovercraft * hovercraft );

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
