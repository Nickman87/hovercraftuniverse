#ifndef HOVERCRAFT_H
#define HOVERCRAFT_H

#include "HovercraftUniverseEntity.h"
#include <tinyxml/tinyxml.h>
#include "Exception.h"
#include "BasicEntityEvent.h"
#include "CollisionEvent.h"

namespace HovUni {

class RacePlayer;
class VisualEvent;

/**
 * The hovercraft entity.
 *
 * @author PJ, Kristof Overdulve
 */ 
class Hovercraft : public HovercraftUniverseEntity {
private:

	/**
	 * The id of the player this hovercraft is for
	 */
	unsigned int mPlayerId;

	/** A reference to the race player */
	RacePlayer* mPlayer;

	/** 
	 * The moving status 
	 */
	BasicEntityEvent mMovingStatus;

	/**
	 * Tilt test 
	 */
	float mTilt;

	/**
	 * The name
	 */
	Ogre::String mDisplayName;

	/**
	 * The name
	 */
	Ogre::String mDescription;

	/**
	 * The default maximum speed
	 */
	Ogre::Real mMaximumSpeed;

	/**
	 * The current speed
	 */
	Ogre::Real mSpeed;

	/**
	 * The mass
	 */
	Ogre::Real mMass;

	/**
	 * The base acceleration without powerups
	 */
	Ogre::Real mAcceleration;

	/**
	 * The steering force
	 */
	Ogre::Real mSteering;

	/** 
	 * Collision state flag. 
	 * If set, the hovercraft is "about to collide" with something. 
	 */
	bool mCollisionState;

	/**
	 * Did this hovercraft finish?
	 * Used to disable controls after finishing.
	 */
	bool mFinished;

	/**
	*	The hovercraft is actually colliding with something -> this event is not null
	*	@author Dirk 
	*/
	CollisionEvent* mCollisionEvent;

public:

	/**
	 * The maximum speed any hovercraft can reach, defined in game.userdatatypes.xml
	 */
	static const Ogre::Real MAXSPEED;

	/**
	 * The category used for hovercrafts
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
	Hovercraft(const Ogre::String& name, unsigned int playerid, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& ogreentity, float processInterval);

	/**
	 * Constructor.
	 * 
	 * @param announcedata
	 */
	Hovercraft( ZCom_BitStream* announcedata );

	/**
	 * load
	 * @param data, xml element that descripes the hovercraft
	 * @throws ParseException
	 */
	void load(TiXmlElement * data) throw(ParseException);

	/**
	 * Destructor
	 */
	~Hovercraft(void);

	/**
	 * Set the player id of the controller of the hovercraft
	 * @param playerid
	 */
	inline void setPlayerId( unsigned int playerid ){
		mPlayerId = playerid;
	}

	/**
	 * Get the player id of the controller of the hovercraft
	 * @param playerid
	 */
	inline const unsigned int getPlayerId() const {
		return mPlayerId;
	}

	/**
	 * Get the moving status
	 * @return the moving status
	 */
	inline const BasicEntityEvent& getMovingStatus() const {
		return mMovingStatus;
	}

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
	 * Get the description
	 * @return description
	 */
	inline Ogre::String getDescription() const {
		return mDescription;
	}

	/**
	 * Set the description
	 * @param description
	 */
	inline void setDescription(const Ogre::String& description) {
		mDescription = description;
	}

	/**
	 * Get the maximum speed
	 * @return maximum speed
	 */
	inline Ogre::Real getMaximumSpeed() const {
		return mMaximumSpeed;
	}

	/**
	 * Get the current speed
	 * @return speed
	 */
	inline Ogre::Real getSpeed() const {
		return mSpeed;
	}

	/**
	 * Set the current speed
	 * @param speed
	 */
	inline void setSpeed(Ogre::Real speed) {
		mSpeed = speed;
	}

	/**
	 * Get the mass
	 * @return mass
	 */
	inline Ogre::Real getMass() const {
		return mMass;
	}

	/**
	 * Set the mass
	 * @param mass
	 */
	inline void setMass(Ogre::Real mass) {
		mMass = mass;
	}

	/**
	 * Get the acceleration
	 * @return acceleration
	 */
	inline Ogre::Real getAcceleration() const {
		return mAcceleration;
	}

	/**
	 * Set the acceleration
	 * @param acceleration
	 */
	inline void setAcceleration(Ogre::Real acceleration) {
		mAcceleration = acceleration;
	}

	/**
	 * Get the steering.
	 *
	 * @return the steering
	 */
	inline Ogre::Real getSteering() const {
		return mSteering;
	}

	/**
	 * Set the steering.
	 *
	 * @param steering the steering
	 */
	inline void setSteering(Ogre::Real steering) {
		mSteering = steering;
	}

	/**
	 * Set the Collision flag
	 * @param	flag	the Collision flag
	 */
	inline void setCollisionState(bool flag) {
		mCollisionState = flag;
	}

	/**
	 * Check the Collision flag
	 * @return the Collision flag
	 */
	inline bool isInCollisionState() const {
		return mCollisionState;
	}

	/**
	 * Check if this hovercraft has finished the race.
	 * @return the finish flag.
	 */
	inline bool isFinished() const {
		return mFinished;
	}

	/**
	 * Set this hovercraft to finished.
	 */
	inline void finished() {
		mFinished = true;
	}

	/**
	 * Update the label
	 */
	void updateLabel();

	/**
	 * Callback to process this entity. This allows to do entity specific processing
	 *(e.g. intermediate actions).
	 *
	 * @param timeSince the time since the last processing of the entity
	 */
	virtual void process(float timeSince);

	/**
	 * Set the race state for the entity
	 *
	 * @param racestate the race state
	 */
	virtual void setRaceState(RaceState* racestate);

	/**
	 * Process visual event
	 * @param e
	 */
	virtual void processVisualEvent ( VisualEvent * e );

	/**
	 * @see NetworkEntity::parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id, ZCom_BitStream* stream, float timeSince)
	 */
	void parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id, ZCom_BitStream* stream, float timeSince);

	/**
	 * Callback to process a controller event at the server that got processed by the 
	 * controller.  Must be overriden since this class in itself has no clue which 
	 * controller properties there are.
	 *
	 * @param event a controller event
	 */
	virtual void processEventsServer(ControllerEvent* cEvent);

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
	virtual void processEventsOther(ControllerEvent* cEvent);

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

	/**
	 * @see NetworkEntity::setAnnouncementData(ZCom_BitStream* stream)
	 */
	void setAnnouncementData(ZCom_BitStream* stream);

	CollisionEvent* getCollisionEvent() const;
};

}

#endif
