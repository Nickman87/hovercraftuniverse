//Havok
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

#include "PortalPhantom.h"

//Havok Framework
#include "HavokEntity.h"
#include "HavokEntityType.h"

//Entities Used
#include "Hovercraft.h"
#include "Portal.h"

namespace HovUni {

PortalPhantom::PortalPhantom(const hkAabb& aabb,  Portal * portal ):
	hkpAabbPhantom( aabb, 0), mPortal(portal)
{
}

PortalPhantom::~PortalPhantom(void)
{
}

void PortalPhantom::addOverlappingCollidable( hkpCollidable* handle )
{
	hkpRigidBody* rb = hkGetRigidBody(handle);

	//if hovercraft send envent to checkpoint
	if ( (rb != HK_NULL) && HavokEntityType::isEntityType(rb,HavokEntityType::CHARACTER ) ) {
		HavokEntity * ch = reinterpret_cast<HavokEntity*>(rb->getUserData());
		Entity * e = ch->getEntity();
		if ( e->getCategory() == Hovercraft::CATEGORY ){
			mPortal->onEnter(dynamic_cast<Hovercraft*>(e));
		}
	}

	hkpAabbPhantom::addOverlappingCollidable( handle );
}

void PortalPhantom::removeOverlappingCollidable( hkpCollidable* handle )
{
	hkpRigidBody* rb = hkGetRigidBody(handle);

	//if hovercraft send envent to checkpoint
	if ( (rb != HK_NULL) && HavokEntityType::isEntityType(rb,HavokEntityType::CHARACTER ) ) {
		HavokEntity * ch = reinterpret_cast<HavokEntity*>(rb->getUserData());
		Entity * e = ch->getEntity();
		if ( e->getCategory() == Hovercraft::CATEGORY ){
			mPortal->onLeave(dynamic_cast<Hovercraft*>(e));
		}
	}

	hkpAabbPhantom::removeOverlappingCollidable( handle );
}

}