#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>

#include "HavokEntityType.h"
#include "PlanetGravityPhantom.h"
#include "PlanetGravityAction.h"
#include "Asteroid.h"

namespace HovUni {

PlanetGravityPhantom::PlanetGravityPhantom( Asteroid * asteroid, hkpRigidBody* body, const hkAabb& aabb )
		: hkpAabbPhantom( aabb, 0), mAsteroid(asteroid), mPlanetBody(body) {
}

PlanetGravityPhantom::~PlanetGravityPhantom(void)
{
}

void PlanetGravityPhantom::addOverlappingCollidable( hkpCollidable* handle )
{
	hkpRigidBody* theBody = hkGetRigidBody( handle );

	if( theBody && (theBody->getMotionType() != hkpMotion::MOTION_FIXED) && (HavokEntityType::getEntityType(theBody) != HavokEntityType::PLANET )  )
	{
		hkBool actionFound = false;

		for( int i = 0; i < theBody->getNumActions(); i++ )
		{
			if( theBody->getAction(i)->getUserData() == PlanetGravityAction::HK_SPHERE_ACTION_ID )
			{
				if( static_cast<PlanetGravityAction*>( theBody->getAction(i) )->accessPhantomId() == reinterpret_cast<hkUlong>( this ) )
				{
					//Do not add action if one already exists for this rigid body
					actionFound = true;
					break;
				}
			}
		}

		if( !actionFound )
		{
			std::cout << "GRAV ADDED " << mAsteroid->getGravity()<< std::endl;
			// Add an action
			PlanetGravityAction* newGravity = new PlanetGravityAction( mPlanetBody, theBody, reinterpret_cast<hkUlong>( this ), mAsteroid->getGravity() );
			theBody->getWorld()->addAction( newGravity );
			newGravity->removeReference();
		}

	}

	hkpAabbPhantom::addOverlappingCollidable( handle );
}

void PlanetGravityPhantom::removeOverlappingCollidable( hkpCollidable* handle )
{
	hkpRigidBody* theBody = hkGetRigidBody( handle );

	if( theBody != HK_NULL )
	{
		for( int i = 0; i < theBody->getNumActions(); i++ )
		{
			// Is there already a PullPlanetGravityAction on the planet?
			if( theBody->getAction(i)->getUserData() == PlanetGravityAction::HK_SPHERE_ACTION_ID )
			{
				if( static_cast<PlanetGravityAction*>( theBody->getAction(i) )->accessPhantomId() == reinterpret_cast<hkUlong>( this ) )
				{
					theBody->getWorld()->removeAction( theBody->getAction(i) );
				}
			}
		}
	}

	hkpAabbPhantom::removeOverlappingCollidable( handle );
}

}