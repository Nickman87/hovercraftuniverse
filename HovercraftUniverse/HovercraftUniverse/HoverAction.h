#ifndef HOVER_ACTION_H
#define HOVER_ACTION_H

#include <Physics/Dynamics/Action/hkpUnaryAction.h>
#include <Physics/Internal/Collide/BroadPhase/hkpBroadPhaseCastCollector.h>
#include <Physics/Collide/Query/CastUtil/hkpWorldRayCastInput.h>

class hkpWorld;

namespace HovUni {

class HavokHovercraft;

class HoverAction : public hkpUnaryAction
{
private:

	/**
	 * The height of the hovering
	 */
	const float mHoveringHeight;

	/**
	 * The magnitude of the gravity force on the character
	 */
	const float mCharacterGravity;

	/**
	 * The hovercraft
	 */
	HavokHovercraft * mHovercraft;

	hkpWorld * mWorld;

public:

	static const hkUlong HK_HOVERACTION_ID = 0x28024501;

	HoverAction(HavokHovercraft * entity, hkpWorld * world);

	~HoverAction(void);

private:

	virtual void applyAction( const hkStepInfo& stepInfo );

	virtual hkpAction* clone( const hkArray<hkpEntity*>& newEntities, const hkArray<hkpPhantom*>& newPhantoms ) const { 
		return HK_NULL; 
	}
};

}

#endif
