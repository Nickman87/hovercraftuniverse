#ifndef ENTITY_H_
#define ENTITY_H_

#include "Controller.h"
#include <OgreVector3.h>
#include "NetworkEntity.h"
#include "ControllerEvent.h"
#include <OgreLogManager.h>
#include "CameraSpring.h"

namespace HovUni {

class EntityPropertyMap;

/**
 * An entity is an object in the game world that has a game state. This state can be modified by the 
 * interaction of players with the game.
 *
 * @author Kristof Overdulve, Olivier Berghmans & Tobias Van Bladel, Nick De Frangh
 */
class Entity: public NetworkEntity {
protected:

	/**
	 * Map with all properties for the entity
	 */
	EntityPropertyMap * mProperties;
	
	/** The unique name of the entity */
	Ogre::String mName;

	/** The category to which this entity belongs */
	Ogre::String mCategory;

	/** The position */
	Ogre::Vector3 mPosition;

	/** Value to check for position change */
	Ogre::Vector3 mLastPosition;

	/** Temp position */
	Ogre::Vector3 mTmpPosition;

	/** The linear velocity (Dirk)*/
	Ogre::Vector3 mVelocity;

	/** The orientation of the entity in the world */
	Ogre::Quaternion mOrientation;

	/** The name of the ogre entity that represents this entity, can be empty */
	Ogre::String mOgreEntity;

	/** Set this in case a label needs to be displayed by an Ogre overlay (Dirk) */
	Ogre::String mLabel;

	/** The controller that the entity polls to change state */
	Controller * mController;

	/** The interval between two processings of the object */
	float mProcessInterval;

	/** The time since last process of the object */
	float mProcessElapsed;

	/** Should the controls be processed or not? */
	static bool mControlsActive;

	/** Interpolator for the position of the hovercraft */
	CameraSpring mSpringInterpolator;

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
	Entity( const Ogre::String& name, const Ogre::String& category, const Ogre::Vector3& position, const Ogre::Vector3& orientation, const Ogre::Vector3& upvector, const Ogre::String& mOgreEntity, float processInterval, unsigned short replicators );

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
	Entity(const Ogre::String& name, const Ogre::String& category, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& mOgreEntity, float processInterval, unsigned short replicators);

	/**
	 * Constructor.
	 *
	 * @param announcementdata the data send by the server
 	 * @param category the category to which this entity belongs
	 * @param replicators the number of replicator to be used
	 */
	Entity ( ZCom_BitStream* announcementdata, const Ogre::String& category, unsigned short replicators );

private:
	/** Do some initialisation */
	void init();

public:

	/**
	 * Destructor.
	 */
	virtual ~Entity();

	/**
	 * Changes the position to the new position.
	 *
	 * @param newPosition is the new position
	 */
	void changePosition(const Ogre::Vector3& newPosition);

	/**
	* Changes the linear velocity to this new value.
	*
	* @param newVelocity is the new linear velocity
	*/
	void changeVelocity(const Ogre::Vector3& newVelocity);

	/**
	 * Changes the orientation to the new orientation.
	 *
	 * @param newOrientation is the new orientation
	 */
	void changeOrientation(const Ogre::Quaternion& newOrientation);

	/**
	 * Sets the controller of the character. This allows for example for live migration between
	 * player controlled characters and AI controlled characters. This operation has as a side
	 * effect that the controller will automatically fetch information from the current entity 
	 * in the future.
	 *
	 * @param controller the controller of the character
	 */
	void setController(Controller * controller);

	/**
	 * Updates the entity.
	 *
	 * @param timeSince the time that elapsed since the last update
	 */
	void update(float timeSince);

	/**
	 * Returns the unique name of this entity.
	 *
	 * @return the unique name
	 */
	Ogre::String getName() const;

	/**
	 * Returns the label (not guaranteed to be unique)
	 * @return	The label
	 */
	Ogre::String getLabel() const;

	/**
	 *	Returns true iff the Entity has a label.
	 *	@return	true iff the Entity has a label
	 */
	bool hasLabel() const;

	/**
	 * Returns the category to which this entity belongs.
	 *
	 * @return the category
	 */
	Ogre::String getCategory() const;

	/**
	 * Returns the position of this entity.
	 *
	 * @return the position
	 */
	Ogre::Vector3 getPosition() const;

	/**
	 * Returns a smoothed position of this entity.
	 *
	 * @return The smoothed position
	 */
	Ogre::Vector3 getSmoothPosition() const;

	/**
	 * Returns the linear velocity of this entity.
	 *
	 * @return the linear velocity 
	 */
	Ogre::Vector3 getVelocity() const;

	/**
	 * Returns the up vector of this entity.
	 *
	 * @return the up vector
	 */
	Ogre::Vector3 getUpVector() const;

	/**
	 * Returns the orientation of this entity.
	 *
	 * @return the orientation
	 */
	Ogre::Vector3 getOrientation() const;

	/**
	 * Returns the unique name of the ogre entity that represents this entity,
	 * Can be empty if no such ogre entity exists.
	 *
	 * @return the name of the ogre entity
	 */
	Ogre::String getOgreEntity() const;

	/**
	 * Returns the orientation of this entity.
	 *
	 * @return the orientation
	 */
	Ogre::Quaternion getQuaternion() const;

	/**
	 * Get the map with all properties of this entity
	 * 
	 * @return map with all properties of this entity
	 */
	EntityPropertyMap * getPropertyMap();

	/**
	 * Make the controls for all entities active.
	 */
	static void setControlsActive();
	
	/**
	 * Make the controls for all entities inactive.
	 */
	static void setControlsInactive();

	/**
	 * If the controls are inactive, make them active.
	 * If the controls are active, make them inactive.
	 */
	static void toggleControlsActive();

protected:
	/**
	 * Callback to process this entity. This allows to do entity specific processing
	 * (e.g. intermediate actions).
	 *
	 * @param timeSince the time since the last processing of the entity
	 */
	virtual void process(float timeSince) = 0;

	/**
	 * Callback to process a controller event at the server that got processed by the 
	 * controller.  Must be overriden since this class in itself has no clue which 
	 * controller properties there are.
	 *
	 * @param event a controller event
	 */
	virtual void processEventsServer(ControllerEvent* cEvent) = 0;

	/**
	 * Callback to process a controller event at the owner that got processed by the 
	 * controller.  Must be overriden since this class in itself has no clue which 
	 * controller properties there are.
	 *
	 * @param event a controller event
	 */
	virtual void processEventsOwner(ControllerEvent* cEvent) = 0;

	/**
	 * Callback to process a controller event at other clients that got processed by the controller.  Must
	 * be overriden since this class in itself has no clue which controller properties 
	 * there are.
	 *
	 * @param event a controller event
	 */
	virtual void processEventsOther(ControllerEvent* cEvent) = 0;

	/**
	 * @see NetworkEntity::setReplication()
	 */
	void setupReplication();

	/**
	 * @see NetworkEntity::parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id, ZCom_BitStream* stream, float timeSince)
	 */
	void parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id, ZCom_BitStream* stream, float timeSince);

	/**
	 * Polls the controller for its current state and processes actions accordingly.
	 *
	 * @param timeSince the time that elapsed since the last process
	 */
	void processController(float timeSince);

	/**
	 * Process a controller event that got processed by the controller and fire the
	 * correct callback (server, owner or others).
	 *
	 * @param event a controller event
	 */
	void processControllerEvents(ControllerEvent* cEvent);

	/**
	 * @see NetworkEntity::setAnnouncementData(ZCom_BitStream* stream)
	 */
	virtual void setAnnouncementData(ZCom_BitStream* stream);
};

}

#endif
