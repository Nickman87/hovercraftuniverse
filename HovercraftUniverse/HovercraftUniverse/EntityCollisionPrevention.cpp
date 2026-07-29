//Havok
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>

#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Dynamics/Phantom/hkpAabbPhantom.h>
#include <Physics/Dynamics/Action/hkpUnaryAction.h>
#include <Physics/Dynamics/Phantom/hkpSimpleShapePhantom.h>

#include "EntityCollisionPrevention.h"

//Havok Framework
#include "Havok.h"
#include "HavokEntity.h"
#include "HavokEntityType.h"

namespace HovUni {

///////////////// SIMPLE VERSION //////////////////////

/**
 * Class that binds an Entity Collision System to the entity
 */
class SimpleEntityCollisionBinder : public hkpUnaryAction {

private:

	hkpAabbPhantom * mPhantom;

	hkReal mOffset;

public:

	/**
	 * Constructor
	 * @param trackedBody
	 * @param phantomToUpdate
	 * @param offset
	 */
	SimpleEntityCollisionBinder( hkpRigidBody* trackedBody, hkpAabbPhantom* phantomToUpdate , hkReal offset ):
		hkpUnaryAction(trackedBody), mPhantom(phantomToUpdate), mOffset(-1.0f*offset) {
	}

	virtual ~SimpleEntityCollisionBinder() {
	}

private:

	virtual void applyAction( const hkStepInfo& stepInfo ){
		// Find the new center to move to
		hkVector4 newCenter = getRigidBody()->getPosition();

		hkRotation ro;
		ro.set(getRigidBody()->getRotation());
		hkVector4 forward = ro.getColumn(2);
		forward.mul4(mOffset);

		newCenter.add4(forward);

		// Grab the halfextents of the phantom's AABB
		hkVector4 halfExtents;
		halfExtents.setSub4( mPhantom->getAabb().m_max, mPhantom->getAabb().m_min );
		halfExtents.mul4( 0.5f );

		// Create a new AABB for the phantom, of the same size
		hkAabb newAabb;
		newAabb.m_min.setSub4( newCenter, halfExtents );
		newAabb.m_max.setAdd4( newCenter, halfExtents );

		// Update the phantom's AABB so it encloses the rigid body
		mPhantom->setAabb( newAabb );
	}

	virtual hkpAction* clone( const hkArray<hkpEntity*>& newEntities, const hkArray<hkpPhantom*>& newPhantoms ) const { 
		return HK_NULL; 
	}

};

/**
 * Phantom used to detect incomming objects, must be placed in front of entity
 */
class SimpleEntityCollisionPhantom : public hkpAabbPhantom
{
private:

	EntityCollision * mCollision;

public:

	/**
	 * Constructor, creates an AABB Collision phantom.
	 *
	 * @param aabb, the bounding box
	 * @param col, the collision system
	 */
	SimpleEntityCollisionPhantom(const hkAabb& aabb, EntityCollision * col):
		hkpAabbPhantom( aabb, 0), mCollision(col) {
	}

	/**
	 * Destructor
	 */
	virtual ~SimpleEntityCollisionPhantom(void){
	}

	/**
	 * @see hkpSimpleShapePhantom
	 */
	virtual void addOverlappingCollidable( hkpCollidable* handle ){
		hkpRigidBody* rb = hkGetRigidBody(handle);

		//if something enters bounding box that is't the planet TODO FIX SOME COOL SYSTEM!!
		if ( (rb != HK_NULL) && !HavokEntityType::isEntityType(rb,HavokEntityType::PLANET ) ) {
			mCollision->onOverlapEnter();	
		}
	}

	/**
	 * @see hkpSimpleShapePhantom
	 */
	virtual void removeOverlappingCollidable( hkpCollidable* handle ){
		hkpRigidBody* rb = hkGetRigidBody(handle);

		//if something leaves bounding box that is't the planet TODO FIX SOME COOL SYSTEM!!
		if ( (rb != HK_NULL) && !HavokEntityType::isEntityType(rb,HavokEntityType::PLANET ) ) {
			mCollision->onOverlapLeave();	
		}
	}
};

SimpleEntityCollision::SimpleEntityCollision ( hkpWorld * world, HavokEntity * entity, hkReal offset, const hkAabb& aabb ):
	mWorld(world)
{
	mWorld->markForWrite();	

	mPhantom = new SimpleEntityCollisionPhantom(aabb, this);
	mWorld->addPhantom(mPhantom);

	mBinder = new SimpleEntityCollisionBinder(entity->getRigidBody(),mPhantom,offset);
	mWorld->addAction(mBinder);

	mWorld->unmarkForWrite();
}

SimpleEntityCollision::~SimpleEntityCollision(){
	mWorld->markForWrite();
	
	mWorld->removeAction(mBinder);
	mBinder->removeReference();
	mBinder = HK_NULL;

	mWorld->removePhantom(mPhantom);
	mPhantom->removeReference();
	mPhantom = HK_NULL;

	mWorld->unmarkForWrite();
}

////////////////   ADVANCED VERSION   //////////////////

/**
 * Class that binds an Entity Collision System to the entity
 */
class AdvancedEntityCollisionBinder : public hkpUnaryAction {

private:

	hkpSimpleShapePhantom * mPhantom;

	hkReal mOffset;

public:

	/**
	 * Constructor
	 * @param trackedBody
	 * @param phantomToUpdate
	 * @param offset
	 */
	AdvancedEntityCollisionBinder( hkpRigidBody* trackedBody, hkpSimpleShapePhantom* phantomToUpdate , hkReal offset ):
		hkpUnaryAction(trackedBody), mPhantom(phantomToUpdate), mOffset(-1.0f*offset) {
	}

	/**
	 * Destructor
	 */
	virtual ~AdvancedEntityCollisionBinder() {
	}

private:

	virtual void applyAction( const hkStepInfo& stepInfo ) {
		// Find the new center to move to
		const hkQuaternion& r = getRigidBody()->getRotation();

		hkRotation rot;
		rot.set(r);

		hkVector4 offset = rot.getColumn(2);
		offset.mul4(mOffset);
		offset.add4(getRigidBody()->getPosition());

		hkTransform t(r,offset);
		mPhantom->setTransform(t);
	}

	virtual hkpAction* clone( const hkArray<hkpEntity*>& newEntities, const hkArray<hkpPhantom*>& newPhantoms ) const { 
		return HK_NULL; 
	}

};

/**
 * Phantom used to detect incomming objects, must be placed in front of entity
 */
class AdvancedEntityCollisionPhantom : public hkpSimpleShapePhantom
{
private:

	/**
	 * The collision object
	 */
	EntityCollision * mCollision;

public:

	/**
	 * Constructor, creates a Collision phantom.
	 *
	 * @param shape, the shape of the detection stystem
	 * @param transform, the initial transform
	 * @param entity, the entity the collision phantom is bound to
	 */
	AdvancedEntityCollisionPhantom(const hkpShape *shape, const hkTransform &transform, EntityCollision * coll) :
		hkpSimpleShapePhantom( shape, transform, 0), mCollision(coll) {
	}

	/**
	 * Destructor
	 */
	~AdvancedEntityCollisionPhantom(void){
	}

	/**
	 * @see hkpSimpleShapePhantom
	 */
	void addOverlappingCollidable( hkpCollidable* handle ){
		hkpRigidBody* rb = hkGetRigidBody(handle);

		//if something enters bounding box that is't the planet
		if ( (rb != HK_NULL) && !HavokEntityType::isEntityType(rb,HavokEntityType::PLANET ) ) {
			mCollision->onOverlapEnter();
		}
	}

	/**
	 * @see hkpSimpleShapePhantom
	 */
	void removeOverlappingCollidable( hkpCollidable* handle ){
		hkpRigidBody* rb = hkGetRigidBody(handle);

		//if something leaves bounding box that is't the planet
		if ( (rb != HK_NULL) && !HavokEntityType::isEntityType(rb,HavokEntityType::PLANET ) ) {
			mCollision->onOverlapLeave();
		}
	}
};


AdvancedEntityCollision::AdvancedEntityCollision ( hkpWorld * world, HavokEntity * entity, hkReal offset, const hkpShape *shape ):
	mWorld(world), mEntity(entity)
{
	mWorld->markForWrite();	

	hkTransform tr;
	tr.setIdentity();

	mPhantom = new AdvancedEntityCollisionPhantom(shape,tr,this);
	mWorld->addPhantom(mPhantom);

	mBinder = new AdvancedEntityCollisionBinder(entity->getRigidBody(),mPhantom,offset);
	mWorld->addAction(mBinder);

	mWorld->unmarkForWrite();
}

AdvancedEntityCollision::~AdvancedEntityCollision(){
	mWorld->markForWrite();
	
	mWorld->removeAction(mBinder);
	mBinder->removeReference();
	mBinder = HK_NULL;

	mWorld->removePhantom(mPhantom);
	mPhantom->removeReference();
	mPhantom = HK_NULL;

	mWorld->unmarkForWrite();
}

}
