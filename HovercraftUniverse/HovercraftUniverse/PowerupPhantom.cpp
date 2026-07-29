//Havok
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

#include "PowerupPhantom.h"

//Havok Framework
#include "HavokEntity.h"
#include "HavokEntityType.h"

//Entities Used
#include "Hovercraft.h"
#include "PowerupSpawn.h"


namespace HovUni {

PowerupPhantom::PowerupPhantom(const hkAabb& aabb, PowerupSpawn * powerupspawn ):
	hkpAabbPhantom( aabb, 0), mPowerupSpawn(powerupspawn)
{
}

PowerupPhantom::~PowerupPhantom(void)
{
}

void PowerupPhantom::addOverlappingCollidable( hkpCollidable* handle )
{
	hkpRigidBody* rb = hkGetRigidBody(handle);

	//if hovercraft send envent to checkpoint
	if ( (rb != HK_NULL) && HavokEntityType::isEntityType(rb,HavokEntityType::CHARACTER ) ) {
		HavokEntity * ch = reinterpret_cast<HavokEntity*>(rb->getUserData());
		Entity * e = ch->getEntity();
		if ( e->getCategory() == Hovercraft::CATEGORY ){
			mPowerupSpawn->onEnter(dynamic_cast<Hovercraft*>(e));
		}
	}

	hkpAabbPhantom::addOverlappingCollidable( handle );
}

void PowerupPhantom::removeOverlappingCollidable( hkpCollidable* handle )
{
	hkpRigidBody* rb = hkGetRigidBody(handle);

	//if hovercraft send envent to checkpoint
	if ( (rb != HK_NULL) && HavokEntityType::isEntityType(rb,HavokEntityType::CHARACTER ) ) {
		HavokEntity * ch = reinterpret_cast<HavokEntity*>(rb->getUserData());
		Entity * e = ch->getEntity();
		if ( e->getCategory() == Hovercraft::CATEGORY ){
			mPowerupSpawn->onEnter(dynamic_cast<Hovercraft*>(e));
		}
	}

	hkpAabbPhantom::removeOverlappingCollidable( handle );
}

}