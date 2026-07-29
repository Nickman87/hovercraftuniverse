//Havok
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

#include "SpeedBoostPhantom.h"

//Havok Framework
#include "HavokEntity.h"
#include "HavokEntityType.h"
#include "BoostAction.h"

//Entities Used
#include "Hovercraft.h"
#include "SpeedBoost.h"

namespace HovUni {

SpeedBoostPhantom::SpeedBoostPhantom(const hkpShape *shape, const hkTransform &transform, SpeedBoost * boost ):
	hkpSimpleShapePhantom( shape, transform, 0), mBoost(boost)
{
}

SpeedBoostPhantom::~SpeedBoostPhantom(void)
{
}

void SpeedBoostPhantom::addOverlappingCollidable( hkpCollidable* handle )
{	
	hkpRigidBody* rb = hkGetRigidBody(handle);

	bool actionfound = false;

	if ( (rb != HK_NULL) && HavokEntityType::isEntityType(rb,HavokEntityType::CHARACTER ) ) {
		HavokEntity * ch = reinterpret_cast<HavokEntity*>(rb->getUserData());
		Entity * e = ch->getEntity();
		
		if ( mBoost->onEnter(e) ){

			//add the action if it doesn't exists
			hkBool actionFound = false;

			for( int i = 0; i < rb->getNumActions(); i++ )
			{
				if( rb->getAction(i)->getUserData() == BoostAction::HK_BOOSTACTION_ID )
				{
					if( static_cast<BoostAction*>( rb->getAction(i) )->getPhantomId() == reinterpret_cast<hkUlong>( this ) ){
						actionFound = true;
						break;
					}
				}
			}

			if( !actionFound )
			{
				// Add an action
				BoostAction * boost = new BoostAction(ch, reinterpret_cast<hkUlong>(this));
				rb->getWorld()->addAction( boost );
				boost->removeReference();
			}
		}
	}

	hkpSimpleShapePhantom::addOverlappingCollidable( handle );
}

void SpeedBoostPhantom::removeOverlappingCollidable( hkpCollidable* handle )
{
	hkpRigidBody* rb = hkGetRigidBody(handle);

	//if hovercraft send envent to checkpoint
	if ( (rb != HK_NULL) && HavokEntityType::isEntityType(rb,HavokEntityType::CHARACTER ) ) {
		HavokEntity * ch = reinterpret_cast<HavokEntity*>(rb->getUserData());
		Entity * e = ch->getEntity();
		mBoost->onLeave(e);

		//remove the action
		for( int i = 0; i < rb->getNumActions(); i++ )
		{
			if( rb->getAction(i)->getUserData() == BoostAction::HK_BOOSTACTION_ID )
			{
				if( static_cast<BoostAction*>( rb->getAction(i) )->getPhantomId() == reinterpret_cast<hkUlong>( this ) )
				{
					rb->getWorld()->removeAction( rb->getAction(i) );
				}
			}
		}
	}

	hkpSimpleShapePhantom::removeOverlappingCollidable( handle );
}

}