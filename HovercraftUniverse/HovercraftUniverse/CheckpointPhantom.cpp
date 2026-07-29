//Havok
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

#include "CheckpointPhantom.h"

//Havok Framework
#include "HavokEntity.h"
#include "HavokEntityType.h"

//Entities Used
#include "Hovercraft.h"
#include "CheckPoint.h"

namespace HovUni {

CheckpointPhantom::CheckpointPhantom(const hkAabb& aabb, CheckPoint * checkpoint ):
	hkpAabbPhantom( aabb, 0), mCheckPoint(checkpoint)
{
}

CheckpointPhantom::~CheckpointPhantom(void)
{
}

void CheckpointPhantom::addOverlappingCollidable( hkpCollidable* handle )
{
	hkpRigidBody* rb = hkGetRigidBody(handle);

	//if hovercraft send envent to checkpoint
	if ( (rb != HK_NULL) && HavokEntityType::isEntityType(rb,HavokEntityType::CHARACTER ) ) {
		HavokEntity * ch = reinterpret_cast<HavokEntity*>(rb->getUserData());
		Entity * e = ch->getEntity();
		if ( e->getCategory() == Hovercraft::CATEGORY ){
			mCheckPoint->onEnter(dynamic_cast<Hovercraft*>(e));
		}
	}

	hkpAabbPhantom::addOverlappingCollidable( handle );
}

void CheckpointPhantom::removeOverlappingCollidable( hkpCollidable* handle )
{
	hkpRigidBody* rb = hkGetRigidBody(handle);

	//if hovercraft send envent to checkpoint
	if ( (rb != HK_NULL) && HavokEntityType::isEntityType(rb,HavokEntityType::CHARACTER ) ) {
		HavokEntity * ch = reinterpret_cast<HavokEntity*>(rb->getUserData());
		Entity * e = ch->getEntity();
		if ( e->getCategory() == Hovercraft::CATEGORY ){
			mCheckPoint->onLeave(dynamic_cast<Hovercraft*>(e));
		}
	}

	hkpAabbPhantom::removeOverlappingCollidable( handle );
}

}