#ifndef HAVOKHOVERCRAFT_H
#define HAVOKHOVERCRAFT_H

#include "HavokEntity.h"
#include "EntityCollisionPrevention.h"

#include <Physics/Utilities/CharacterControl/CharacterRigidBody/hkpCharacterRigidBody.h>
#include <Physics/Utilities/CharacterControl/StateMachine/hkpDefaultCharacterStates.h>
#include <Physics/Utilities/Serialize/hkpHavokSnapshot.h>
#include <Physics/Collide/Shape/Convex/Capsule/hkpCapsuleShape.h>

namespace HovUni {

class Hovercraft;
class HavokHovercraft;


/**
 * Tests for the simple collision prevention for the hovercraft
 */

class SimpleTest : public SimpleEntityCollision {

public:

	SimpleTest( hkpWorld * world, HavokHovercraft * hover );

	/**
	 * @see HovUni::EntityCollision
	 */
	virtual void onOverlapEnter( );
	/**
	 * @see HovUni::EntityCollision
	 */
	virtual void onOverlapLeave( );
};


class AdvancedTest : public AdvancedEntityCollision {

public:

	AdvancedTest( hkpWorld * world, HavokHovercraft * hover );

	/**
	 * @see HovUni::EntityCollision
	 */
	virtual void onOverlapEnter( );

	/**
	 * @see HovUni::EntityCollision
	 */
	virtual void onOverlapLeave( );
};


class HavokHovercraft : public HavokEntity
{
private:

	EntityCollision * mCollisionTest;

	hkPackfileReader::AllocatedData* mLoadedData;

	hkpPhysicsData * mPhysicsData;

	/**
	 * The filename from which the hovercraft physic should be read
	 */
	hkString mFilename;

	hkString mEntityName;

	Hovercraft * mEntity;
	
 	hkpCharacterRigidBody* mCharacterRigidBody;

	hkpCharacterContext * mCharacterContext;

	// Current world vectors. This changes as the simulation progresses.
 	hkVector4 mUp;
 	hkVector4 mSide;
	hkVector4 mForward;	

	const float mRotationDelta;
	const float mSpeedDamping;
	const float mCharacterGravity;

	/** Counter of the current amount of active collision events */
	int mCollisionCounter;

public:

	/**
	 * Constructor
	 * @paam world, the physics world
	 * @param entity, the hovercraft game entity
	 * @param filename, the filename of the hovercraft
	 * @param entityname, the name of the hovercraft
	 */
	HavokHovercraft(hkpWorld * world, Hovercraft * entity, const hkString& filename, const hkString& entityname );

	~HavokHovercraft(void);

	/**
	 * Load the hovercraft at given position with given orientation
	 * 
	 * @param position
	 * @param rotation
	 */
	virtual void load(const hkVector4& position, const hkQuaternion& rotation);

	/**
	 * Get the up vector of the hovercraft
	 * @return the up vector
	 */
	virtual const hkVector4& getUp() const;

	/**
	 * Get the position of the character
	 * @return the position of the character
	 */
	virtual const hkVector4& getPosition() const;

	/**
	* Get the linear velocity of the character (Dirk, 24/03/2010)
	* @reurn the linear velocity of the character
	*/
	virtual const hkVector4& getVelocity() const;

	/**
	 * Get the orientation of the character
	 * @return the orientation of the character
	 */
	virtual const hkQuaternion& getOrientation() const;

	/**
	 * Get the rigid body that is used for the entity
	 * @return the rigid body used for the entity
	 */
	virtual hkpRigidBody * getRigidBody() const;

	/**
	 * Get the entity
	 * @return entity
	 */
	virtual Entity * getEntity();

	/**
	 * Update the character based on movement
	 */
	virtual void update();

	/**
	 * Update the up vector
	 * @param up vector
	 */
	virtual void updateUp( const hkVector4& newUp);
	
	/** Increases the active collision state counter by 1. */
	void increaseCollisionCounter();
	/** Decreases the active collision state counter by 1. */
	void decreaseCollisionCounter();
};

}

#endif