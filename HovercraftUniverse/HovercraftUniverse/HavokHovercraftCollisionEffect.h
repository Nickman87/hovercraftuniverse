#ifndef HAVOKHOVERCRAFTCOLLISIONEFFECT_H
#define HAVOKHOVERCRAFTCOLLISIONEFFECT_H

#include <Physics/Dynamics/Action/hkpUnaryAction.h>
#include <Physics/Dynamics/Collide/hkpCollisionListener.h>

namespace HovUni {

class HavokHovercraft;
class HavokHovercraftCollisionEffect;

/**
 * A collision listener for the hovercraft, this class is a helper for the collision bump action
 * @author Pieter-Jan Pintens
 */
class HovercraftCollisionListener : public hkpCollisionListener {

private:

	HavokHovercraftCollisionEffect * mCollisionEffect;

public:

	/**
	 * Constructor
	 * @param collsion effect action
	 */ 
	HovercraftCollisionListener(HavokHovercraftCollisionEffect * mCollisionEffect);

	void  contactPointAddedCallback (hkpContactPointAddedEvent &e);

	void  contactPointConfirmedCallback (hkpContactPointConfirmedEvent &e);
	 
	void  contactPointRemovedCallback (hkpContactPointRemovedEvent &e);
	 
	void  contactProcessCallback (hkpContactProcessEvent &e);
};

/**
 * The action the will cause the Hovercraft to bump on hits
 * @author Pieter-Jan Pintens
 */
class HavokHovercraftCollisionEffect: public hkpUnaryAction
{
private:

	friend class HovercraftCollisionListener;
	
	/**
	 * The listener
	 */
	HovercraftCollisionListener * mListener;
	
	/**
	 * The hovercraft
	 */
	HavokHovercraft * mHovercraft;

	/**
	 * Current bounce vector
	 */
	hkVector4 mBounceVector;

	/**
	 * Current bounce position
	 */
	hkVector4 mNormal;

	/**
	 * Current bounce normal
	 */
	hkVector4 mPosition;

	/** 
	 * True if collision occured
	 */
	bool mHasCollision;

public:

	/**
	 * Constructor
	 * @param hovercraft
	 */
	HavokHovercraftCollisionEffect(HavokHovercraft * hovercraft);

	~HavokHovercraftCollisionEffect(void);

	virtual void applyAction( const hkStepInfo& stepInfo );

	virtual hkpAction* clone( const hkArray<hkpEntity*>& newEntities, const hkArray<hkpPhantom*>& newPhantoms ) const { 
		return HK_NULL; 
	}
};

}

#endif