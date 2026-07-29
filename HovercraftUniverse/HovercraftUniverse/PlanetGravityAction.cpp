#include "PlanetGravityAction.h"
#include "HavokEntityType.h"
#include "HoverCraftUniverseWorld.h"
#include "Havok.h"

#include <Physics/Collide/Query/Collector/PointCollector/hkpClosestCdPointCollector.h>
#include <Physics/Collide/Agent/hkpProcessCollisionInput.h>
#include <Physics/Collide/Dispatch/hkpCollisionDispatcher.h>

namespace HovUni {

PlanetGravityAction::PlanetGravityAction(hkpRigidBody* planetBody, hkpRigidBody* satellite, hkUlong phantomId, hkReal maxAcceleration):
			hkpUnaryAction(satellite), mPlanetBody(planetBody), mPhantomId(phantomId), mGravityForce(maxAcceleration)
 {
	setUserData(HK_SPHERE_ACTION_ID);
}

void PlanetGravityAction::applyAction(const hkStepInfo& stepInfo)
{
	hkpRigidBody* rb = getRigidBody();

	// Sets forceDir to be the difference of m_center and rb->getCenterOfMassInWorld().
	hkVector4 forceDir;

	hkpCollisionInput input = *getWorld()->getCollisionInput();
	hkpClosestCdPointCollector collector;
	hkpShapeType a = rb->getCollidable()->getShape()->getType();
	input.m_tolerance = 1.0f;

	do {
		// Use mHullCollidable for gravity calculations if available.
		// GetClosestPoints() should provide a gravity vector
		/*if (mHullCollidable == HK_NULL) {*/
			hkpShapeType b = mPlanetBody->getCollidable()->getShape()->getType();
			hkpCollisionDispatcher::GetClosestPointsFunc getClosestPoints = getWorld()->getCollisionDispatcher()->getGetClosestPointsFunc(a, b);
			getClosestPoints(*rb->getCollidable(), *mPlanetBody->getCollidable(), input, collector);
		/*} else {
			hkpShapeType b = mHullCollidable->getShape()->getType();
			hkpCollisionDispatcher::GetClosestPointsFunc getClosestPoints = getWorld()->getCollisionDispatcher()->getGetClosestPointsFunc(a, b);
			getClosestPoints(*rb->getCollidable(), *mHullCollidable, input, collector);	
		}*/

		// Grow the tolerance until we get a hit.
		input.m_tolerance = input.m_tolerance * 2.0f;
	} while (!collector.hasHit());

	// Flip the normal of the getClosestPoints() result
	forceDir.setMul4(-1.0f, collector.getHitContact().getNormal());
	forceDir.normalize3();
	forceDir.zeroElement(3);

	// Set force to be direction multiplied by magnitude multiplied by mass.
	hkVector4 force;

	if (HavokEntityType::getEntityType(rb) == HavokEntityType::CHARACTER) {
		HavokEntity * character = reinterpret_cast<HavokEntity*>(rb->getUserData());
		if (character != HK_NULL) {
			hkVector4 oldUp = character->getUp();

			hkVector4 newUp(forceDir);
			newUp.setNeg4(forceDir);
			newUp.normalize3();

			float speed = 0.8f;
			hkVector4 inter(oldUp(0) + (newUp(0) - oldUp(0)) * speed, oldUp(1) + (newUp(1) - oldUp(1)) * speed, oldUp(2) + (newUp(2) - oldUp(2)) * speed);
			inter.normalize3();
			character->updateUp(inter);
		}
	}

	

	force.setMul4(rb->getMass() * mGravityForce, forceDir);
	rb->applyForce(stepInfo.m_deltaTime, force);
}

}

