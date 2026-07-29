#ifndef ENTITY_COLLISION_PREVENTION_H
#define ENTITY_COLLISION_PREVENTION_H

class hkpWorld;
class hkpShape;

namespace HovUni {

class HavokEntity;

/**
 * Interface that should be used for collision detection.
 */
class EntityCollision {

public:

	EntityCollision(){
	}

	virtual ~EntityCollision(){
	}

	/**
	 * Called when an object overlap detected with the phantom shape
	 *
	 * @param ??
	 */
	virtual void onOverlapEnter( ) = 0;

	/**
	 * Called when an object overlap is removed with the phantom shape
	 *
	 * @param ??
	 */
	virtual void onOverlapLeave( ) = 0;	
};

////////////////////////////////////////////////////////////
// There are two systems that implement this interface, a simple one and an advanced one:
//
//	SIMPLE
//	The simple system uses an axis aligned bounding box that is positioned far ahead of the car.
//	This shape will follow only the movement of the car allowing detection of objects that
//	are positioned (far) in front of the car.
//
//	#####				-------
//	#CAR#				|SHAPE|
//	#####				-------
//
//	ADVANCED
//	The advanced system uses a shape instead of an axis aligned bounding box.
//	This shape will follow the movement AND rotation of the car allowing detection of objects that
//	are positioned right in front of the car (far aswell close).
//
//	#####  -------------------
//	#CAR#  |	  SHAPE		 |
//	#####  -------------------
//
//	The shapes can be seen in the visual debugger, use it if not clear!
//
////////////////////////////////////////////////////////////

/**
 * Forward declaration of internal classes
 */
class SimpleEntityCollisionBinder;
class SimpleEntityCollisionPhantom;

/**
 * The simple implementation of the collision system
 * @author Pieter-Jan Pintens
 */
class SimpleEntityCollision : public EntityCollision {

private:

	//The world
	hkpWorld * mWorld;

	//The binder
	SimpleEntityCollisionBinder * mBinder;

	//The phantom
	SimpleEntityCollisionPhantom * mPhantom;

public:

	/**
	 * Constructor
	 *
	 * @param world, the physics world
	 * @param entity, the entity the detection system should bind to
	 * @param offset, the distance from center of the entity
	 * @param aabb, the bounding box
	 */
	SimpleEntityCollision ( hkpWorld * world, HavokEntity * entity, hkReal offset, const hkAabb& aabb );

	/**
	 * Destructor
	 */
	~SimpleEntityCollision();

	/**
	 * @see HovUni::EntityCollision
	 */
	virtual void onOverlapEnter( ){
	}

	/**
	 * @see HovUni::EntityCollision
	 */
	virtual void onOverlapLeave( ){
	}

};



/**
 * Forward declaration of internal classes
 */
class AdvancedEntityCollisionBinder;
class AdvancedEntityCollisionPhantom;

/**
 * The advanced implementation of the collision system
 * @author Pieter-Jan Pintens & Dirk Delahaye
 */
class AdvancedEntityCollision : public EntityCollision {

protected:

	//The world
	hkpWorld * mWorld;

	/** Havok entity to track */
	HavokEntity * mEntity;

	//The binder
	AdvancedEntityCollisionBinder * mBinder;

	//The phantom
	AdvancedEntityCollisionPhantom * mPhantom;

public:

	/**
	 * Constructor
	 *
	 * @param world, the physics world
	 * @param entity, the entity the detection system should bind to
	 * @param shape, the shape for the phantom
	 * @param transform, the initial transform
	 */
	AdvancedEntityCollision ( hkpWorld * world, HavokEntity * entity, hkReal offset, const hkpShape *shape );

	/**
	 * Destructor
	 */
	~AdvancedEntityCollision();

	/**
	 * @see HovUni::EntityCollision
	 */
	virtual void onOverlapEnter( ){
	}

	/**
	 * @see HovUni::EntityCollision
	 */
	virtual void onOverlapLeave( ){
	}
};

}

#endif