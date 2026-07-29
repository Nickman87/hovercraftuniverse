#ifndef CHECKPOINT_H
#define CHECKPOINT_H

#include "HovercraftUniverseEntity.h"
#include <tinyxml/tinyxml.h>
#include "Exception.h"
#include <list>

namespace HovUni {

class Hovercraft;

/**
 * @author PJ
 */ 
class CheckPoint : public HovercraftUniverseEntity {
private:

	/**
	 * The name
	 */
	Ogre::String mDisplayName;

	/**
	 * The number
	 */
	Ogre::int32 mNumber;

public:

	/**
	 * The category used for checkpoints
	 */
	static const Ogre::String CATEGORY;

	/**
	 * Constructor.
	 *
	 * @param name The name of the checkpoint
	 * @param position
	 * @param quaternion
	 * @param processInterval The process interval
	 */
	CheckPoint(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& ogreentity, float processInterval);

	/**
	 * Constructor.
	 * 
	 * @param announcedata
	 */
	CheckPoint( ZCom_BitStream* announcedata );

	/**
	 * Load
	 * @param data, xml element that descripes the checkpoint
	 * @throws ParseException
	 */
	void load(TiXmlElement * data) throw(ParseException);

	/**
	 * Destructor
	 */
	~CheckPoint(void);

	/**
	 * Get the name
	 * @return name
	 */
	inline Ogre::String getDisplayName() const {
		return mDisplayName;
	}

	/**
	 * Set the name
	 * @param name
	 */
	inline void setisplayName(const Ogre::String& name) {
		mDisplayName = name;
	}

	/**
	 * Get the number
	 * @return number
	 */
	inline Ogre::int32 getNumber() const {
		return mNumber;
	}

	/**
	 * Set the number
	 * @param number
	 */
	inline void setNumber(Ogre::int32 number) {
		mNumber = number;
	}

	/**
	 * Called when given hovercraft enters the checkpoint bounding box
	 * @param hovercraft
	 */
	void onEnter ( Hovercraft * hovercraft );

	/**
	 * Called when given hovercraft leaves the checkpoint bounding box
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
