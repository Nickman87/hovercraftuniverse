#ifndef UPDATEPOSITIONACTION_H
#define UPDATEPOSITIONACTION_H

#include <Physics/Dynamics/Action/hkpUnaryAction.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Dynamics/Entity/hkpEntityActivationListener.h>



namespace HovUni {

class HovercraftUniverseEntity;

/**
 * This action will keep the position of a static body entity equal to the position of a havok rigid body
 * @author Pieter-Jan Pintens
 */
class UpdatePositionAction : public hkpUnaryAction, public hkpEntityActivationListener
{
public:

	/**
	 * Constructor
	 */
	UpdatePositionAction( hkpRigidBody * body, HovercraftUniverseEntity * gamebody, hkReal delta = 0.01f );

	~UpdatePositionAction(void);

	virtual void  entityDeactivatedCallback (hkpEntity *entity); 
   
	virtual void  entityActivatedCallback (hkpEntity *entity);

private:

	hkReal mDelta;

	HovercraftUniverseEntity * mStaticBody;

	hkVector4 mPreviousPosition;

	hkQuaternion mPreviousOrientation;

private:

	void update();

	virtual void applyAction( const hkStepInfo& stepInfo );

	virtual hkpAction* clone( const hkArray<hkpEntity*>& newEntities, const hkArray<hkpPhantom*>& newPhantoms ) const { 
		return HK_NULL; 
	}
};

}

#endif