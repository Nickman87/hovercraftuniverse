#ifndef HK_PHANTOM_TRACK_ACTION_H
#define HK_PHANTOM_TRACK_ACTION_H

#include <Physics/Dynamics/Action/hkpUnaryAction.h>

class hkpAabbPhantom;

namespace HovUni {

// Just in case you want the phantom to be updated with a body
// It will also then update the center for any overlapping bodies with gravity actions associated 
// with the body. For now it will just center the Aabb around the body's position but could be extended
// for an offset transform
class PhantomTrackAction: public hkpUnaryAction
{
	public:
		// Constructor takes the rigid body it should track and the phantom it should update
		PhantomTrackAction( hkpRigidBody* trackedBody, hkpAabbPhantom* phantomToUpdate );
	private:
		// Move the phantom m_phantom so it tracks trackedBody
		virtual void applyAction( const hkStepInfo& stepInfo );

		virtual hkpAction* clone( const hkArray<hkpEntity*>& newEntities, const hkArray<hkpPhantom*>& newPhantoms ) const 
		{ 
			return HK_NULL; 
		}

		// Phantom to track the body.
		 hkpAabbPhantom* m_phantom;
};

}

#endif // HK_PHANTOM_TRACK_ACTION_H
