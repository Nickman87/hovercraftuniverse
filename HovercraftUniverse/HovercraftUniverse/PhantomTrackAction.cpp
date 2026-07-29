#include "PhantomTrackAction.h"
#include <Physics/Dynamics/Phantom/hkpAabbPhantom.h>


namespace HovUni {

PhantomTrackAction::PhantomTrackAction( hkpRigidBody* trackedBody, hkpAabbPhantom* phantomToUpdate ) :
	hkpUnaryAction( trackedBody ), m_phantom(phantomToUpdate)
{
}

void PhantomTrackAction::applyAction( const hkStepInfo& stepInfo )
{
	// Find the new center to move to
	const hkVector4& newCenter = getRigidBody()->getPosition();

	// Grab the halfextents of the phantom's AABB
	hkVector4 halfExtents;
	halfExtents.setSub4( m_phantom->getAabb().m_max, m_phantom->getAabb().m_min );
	halfExtents.mul4( 0.5f );

	// Create a new AABB for the phantom, of the same size
	hkAabb newAabb;
	newAabb.m_min.setSub4( newCenter, halfExtents );
	newAabb.m_max.setAdd4( newCenter, halfExtents );

	// Update the phantom's AABB so it encloses the rigid body
	m_phantom->setAabb( newAabb );
}

}

