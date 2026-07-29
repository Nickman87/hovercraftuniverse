//Havok
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>


#include "BoostAction.h"

#include "HavokEntity.h"
#include "Entity.h"
#include "Hovercraft.h"
#include "BoostProperty.h"

namespace HovUni {

BoostAction::BoostAction(HavokEntity * entity, hkLong PhantomId):
	hkpUnaryAction(entity->getRigidBody()), mEntity(entity), mPhantomId(PhantomId)
{
	this->setUserData(HK_BOOSTACTION_ID);
}

BoostAction::~BoostAction(void)
{
}

void BoostAction::applyAction( const hkStepInfo& stepInfo ){

	Entity* ent = mEntity->getEntity();
	EntityProperty * prop = ent->getPropertyMap()->getProperty(BoostProperty::KEY);

	BoostProperty * boost = dynamic_cast<BoostProperty *>(prop);

	if( boost ){
		const Ogre::Vector3& boostdir = boost->getDirection();
		hkVector4 force;
		force.set(boostdir.x,boostdir.y,boostdir.z);
		hkReal newboost = boost->getBoost() + boost->getGain();				
		force.mul4(newboost);
		getRigidBody()->applyLinearImpulse(force);
		boost->setBoost(newboost);
	}
}

}
