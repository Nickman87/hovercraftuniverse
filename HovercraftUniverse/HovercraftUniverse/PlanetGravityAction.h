#ifndef PLANETGRAVITYACTION_H
#define PLANETGRAVITYACTION_H

#include <Physics/Dynamics/Action/hkpUnaryAction.h>

namespace HovUni {

/**
 * Bodies with this action will have a force applied to them toward the surface of the planet
 */
class PlanetGravityAction: public hkpUnaryAction
{
	private:

		/**
		 * Planet to which we are being attracted
		 */
		hkpRigidBody* mPlanetBody;

		/**
		 * Phantom ID, if a phantom has been associated with this action then we have this ID so that
		 * we can search through a body's actions and update values as necessary
		 */
		hkUlong mPhantomId;

		/**
		 * Max Acceleration used for objects inside innerRadius
		 */
		hkReal mGravityForce;

	public:

		static const hkUlong HK_SPHERE_ACTION_ID = 0x28021978;
		
		/**
		 * Constructor
		 * @param planetBody
		 * @param satellite
		 * @param phantomId, 0 for no phantomID
		 * @param maxAcceleration
		 */
		PlanetGravityAction( hkpRigidBody* planetBody, hkpRigidBody* satellite, hkUlong phantomId, hkReal maxAcceleration );

		/**
		 * Get the phantom ID
		 * @return phantom ID
		 */
		inline hkUlong& accessPhantomId() {
			return mPhantomId;
		}

	private:

		virtual void applyAction( const hkStepInfo& stepInfo );

		virtual hkpAction* clone( const hkArray<hkpEntity*>& newEntities, const hkArray<hkpPhantom*>& newPhantoms ) const { return HK_NULL; }
};

}

#endif