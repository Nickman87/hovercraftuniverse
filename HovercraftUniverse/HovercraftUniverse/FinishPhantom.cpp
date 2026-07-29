//Havok
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

#include "FinishPhantom.h"

//Havok Framework
#include "HavokEntity.h"
#include "HavokEntityType.h"

//Entities Used
#include "Hovercraft.h"
#include "Finish.h"


namespace HovUni {

FinishPhantom::FinishPhantom(const hkAabb& aabb, Finish * finish ):
	hkpAabbPhantom( aabb, 0), mFinish(finish)
{
}

FinishPhantom::~FinishPhantom(void)
{
}

void FinishPhantom::addOverlappingCollidable( hkpCollidable* handle )
{
	hkpRigidBody* rb = hkGetRigidBody(handle);

	//if hovercraft send envent to finish
	if ( (rb != HK_NULL) && HavokEntityType::isEntityType(rb,HavokEntityType::CHARACTER ) ) {
		HavokEntity * ch = reinterpret_cast<HavokEntity*>(rb->getUserData());
		Entity * e = ch->getEntity();
		if ( e->getCategory() == Hovercraft::CATEGORY ){
			mFinish->onEnter(dynamic_cast<Hovercraft*>(e));
		}
	}

	hkpAabbPhantom::addOverlappingCollidable( handle );
}

void FinishPhantom::removeOverlappingCollidable( hkpCollidable* handle )
{
	hkpRigidBody* rb = hkGetRigidBody(handle);

	//if hovercraft send envent to finish
	if ( (rb != HK_NULL) && HavokEntityType::isEntityType(rb,HavokEntityType::CHARACTER ) ) {
		HavokEntity * ch = reinterpret_cast<HavokEntity*>(rb->getUserData());
		Entity * e = ch->getEntity();
		if ( e->getCategory() == Hovercraft::CATEGORY ){
			mFinish->onLeave(dynamic_cast<Hovercraft*>(e));
		}
	}

	hkpAabbPhantom::removeOverlappingCollidable( handle );
}

}