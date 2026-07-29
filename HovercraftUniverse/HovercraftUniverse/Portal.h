#ifndef PORTAL_H
#define PORTAL_H

#include "HovercraftUniverseEntity.h"
#include <tinyxml/tinyxml.h>
#include "Exception.h"

namespace HovUni {

class Hovercraft;

/**
 * Portal 
 * A portal is a teleportation mechanism that ports objects from begin to end
 * @author PJ	
 */
class Portal : public HovercraftUniverseEntity {
private:

	/**
	 * Name of the portal
	 */
	Ogre::String mDisplayName;

	/**
	 * Start of the portal
	 */
	Ogre::Vector3 mStart;

	/**
	 * End of the portal
	 */
	Ogre::Vector3 mEnd;

	/**
	 * Time before teleport is done
	 */
	Ogre::Real mTime;

public:

	/**
	 * The category used for portals
	 */
	static const Ogre::String CATEGORY;

	/**
	 * Constructor.
	 *
	 * @param name The name of the portal
	 * @param position
	 * @param quaternion
	 * @param processInterval The process interval
	 */
	Portal(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& ogreentity, float processInterval);

	/**
	 * Constructor.
	 * 
	 * @param announcedata
	 */
	Portal( ZCom_BitStream* announcedata );

	/**
	 * load
	 * @param data
	 * @throws ParseException
	 */
	void load(TiXmlElement * data) throw(ParseException);

	/**
	 * Destructor
	 */
	~Portal(void);

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
	inline void setDisplayName(const Ogre::String& name) {
		mDisplayName = name;
	}

	/**
	 * Get start point of the portal
	 * @return start
	 */
	inline const Ogre::Vector3& getStart() const {
		return mStart;
	}

	/**
	 * Get start point of the portal
	 * @return start
	 */
	inline Ogre::Vector3& getStart() {
		return mStart;
	}

	/**
	 * Set the start point of the portal
	 * @param start
	 */
	void setStart(const Ogre::Vector3& start){
		mStart = start;
	}

	/**
	 * Get end point of the portal
	 * @return end
	 */
	inline const Ogre::Vector3& getEnd() const {
		return mEnd;
	}

	/**
	 * Get end point of the portal
	 * @return end
	 */
	inline Ogre::Vector3& getEnd() {
		return mEnd;
	}

	/**
	 * Set the end point of the portal
	 * @param end
	 */
	void setEnd(const Ogre::Vector3& end){
		mEnd = end;
	}

	/**
	 * Called when given hovercraft enters the portal bounding box
	 * @param hovercraft
	 */
	void onEnter ( Hovercraft * hovercraft );

	/**
	 * Called when given hovercraft leaves the portal bounding box
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