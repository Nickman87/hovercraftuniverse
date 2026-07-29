#include "HoverAction.h"
#include "HavokHovercraft.h"
#include "DedicatedServer.h"

#include <Physics/Dynamics/World/hkpWorld.h>
#include <Physics/Collide/Shape/Query/hkpShapeRayCastInput.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastOutput.h>
#include <Physics/Internal/Collide/BroadPhase/hkpBroadPhase.h>
#include <Physics/Collide/Shape/Query/hkpRayHitCollector.h>

#include <Common/Visualize/hkDebugDisplay.h>

#include <Physics/Collide/Query/CastUtil/hkpSimpleWorldRayCaster.h>

#include "HavokEntityType.h"
#include "EntityManager.h"
#include "Hovercraft.h"
#include "Asteroid.h"

#include <iostream>

namespace HovUni {

// This class links the output of the broadphase raycaster to the narrowphase raycaster.
class PlanetRayCastCallback : public hkpSimpleWorldRayCaster
{
private:

	hkReal& mGravity;

public:

	PlanetRayCastCallback( hkReal& gravity ):
		 hkpSimpleWorldRayCaster(), mGravity(gravity){
	}

protected:


	// the function which is called every time the broadphase raycaster hits the aabb of an
	// object. This implementation checks the type of object and calls object->raycast if
	// necessary
	virtual	hkReal addBroadPhaseHandle( const hkpBroadPhaseHandle* broadPhaseHandle, int castIndex ){
		//in this method one needs to find the actual intersection with the ray, this is updated so only planets are checked

		//check if entity
		const hkpCollidable* col = static_cast<hkpCollidable*>( static_cast<const hkpTypedBroadPhaseHandle*>(broadPhaseHandle)->getOwner() );
		if ( col->getType() != hkpWorldObject::BROAD_PHASE_ENTITY ){
			return 1.0f; //stop if not
		}
			
		//check if planet
		const hkpEntity* ent = static_cast<hkpEntity*>(col->getOwner());
		if ( HavokEntityType::getEntityType(ent) != HavokEntityType::PLANET ){
			return 1.0f; //stop if not
		}

		Asteroid* asteroid = reinterpret_cast<Asteroid*>(ent->getUserData());
		mGravity = asteroid->getGravity();

		//get the real collision point
		return  hkpSimpleWorldRayCaster::addBroadPhaseHandle(broadPhaseHandle,castIndex); //stop
	}
};


//The implementation

HoverAction::HoverAction(HavokHovercraft * entity, hkpWorld * world):
	hkpUnaryAction(entity->getRigidBody()), mHovercraft(entity), mWorld(world),
	mHoveringHeight(DedicatedServer::getEngineSettings()->getValue<float>("Hovering", "Height", 2.5f)),
	mCharacterGravity(DedicatedServer::getEngineSettings()->getValue<float>("Havok", "CharacterGravity", 20.0f))
{
}

HoverAction::~HoverAction(void)
{
}

void HoverAction::applyAction( const hkStepInfo& stepInfo ){
	const hkReal NEGRAYLENGTH = -2.0f * mHoveringHeight;	//double of hoverheight
	const hkReal POSRAYLENGTH = 0.1f;

	hkVector4 from = mHovercraft->getUp();
	from.mul4(POSRAYLENGTH);
	from.add4(mHovercraft->getRigidBody()->getPosition());

	hkVector4 to = mHovercraft->getUp();
	to.mul4(NEGRAYLENGTH);
	to.add4(mHovercraft->getRigidBody()->getPosition());

	HK_DISPLAY_LINE(from,to, hkColor::AZURE );
	
	hkpWorldRayCastInput input;
	input.m_from = from;
	input.m_to = to;

	hkpWorldRayCastOutput output;
	hkReal currentgravity = 0;


	PlanetRayCastCallback caster(currentgravity);
	caster.castRay(*mWorld->getBroadPhase(),input,mWorld->getCollisionFilter(),output);

	if (output.hasHit()){

		hkReal dist = -1.0f * output.m_hitFraction * NEGRAYLENGTH;

		//only hover if hovercraft sufficienttly close to planet
		if ( dist < (mHoveringHeight) ){
		
			// hovering
			hkVector4 hover;

			hover.setMul4(mHovercraft->getRigidBody()->getMass() * currentgravity * (1.0f/dist), mHovercraft->getUp());
			mHovercraft->getRigidBody()->applyForce(stepInfo.m_deltaTime, hover);
		}
		

	}
}

}