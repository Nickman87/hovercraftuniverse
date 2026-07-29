//Havok
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

#include "StartPhantom.h"

//Havok Framework
#include "HavokEntity.h"
#include "HavokEntityType.h"

//Entities Used
#include "Hovercraft.h"
#include "Start.h"


namespace HovUni {

StartPhantom::StartPhantom(const hkAabb& aabb, Start * start ):
	hkpAabbPhantom( aabb, 0), mStart(start)
{
}

StartPhantom::~StartPhantom(void)
{
}

void StartPhantom::addOverlappingCollidable( hkpCollidable* handle )
{
	hkpRigidBody* rb = hkGetRigidBody(handle);

	//if hovercraft send envent to checkpoint
	if ( (rb != HK_NULL) && HavokEntityType::isEntityType(rb,HavokEntityType::CHARACTER ) ) {
		HavokEntity * ch = reinterpret_cast<HavokEntity*>(rb->getUserData());
		Entity * e = ch->getEntity();
		if ( e->getCategory() == Hovercraft::CATEGORY ){
			mStart->onEnter(dynamic_cast<Hovercraft*>(e));
		}
	}

	hkpAabbPhantom::addOverlappingCollidable( handle );
}

void StartPhantom::removeOverlappingCollidable( hkpCollidable* handle )
{
	hkpRigidBody* rb = hkGetRigidBody(handle);

	//if hovercraft send envent to checkpoint
	if ( (rb != HK_NULL) && HavokEntityType::isEntityType(rb,HavokEntityType::CHARACTER ) ) {
		HavokEntity * ch = reinterpret_cast<HavokEntity*>(rb->getUserData());
		Entity * e = ch->getEntity();
		if ( e->getCategory() == Hovercraft::CATEGORY ){
			mStart->onLeave(dynamic_cast<Hovercraft*>(e));
		}
	}

	hkpAabbPhantom::removeOverlappingCollidable( handle );
}

}