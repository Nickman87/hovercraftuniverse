#ifndef BOOSTACTION_H
#define BOOSTACTION_H

#include <Physics/Dynamics/Action/hkpUnaryAction.h>

namespace HovUni {

class HavokEntity;

/**
 * This action will be triggered when boost is triggered.
 * @author Pieter-Jan Pintens
 */
class BoostAction : public hkpUnaryAction
{
private:

	/**
	 * The entity this boost works on
	 */
	HavokEntity * mEntity;

	/**
	 * The phantom ID
	 */
	hkUlong mPhantomId;

public:

	static const hkUlong HK_BOOSTACTION_ID = 0x28024500;

	/**
	 * Constructor
	 *
	 * @param entity, the entity on which the boost works
	 * @param phantomid, the phantom that trigered the boost
	 */
	BoostAction( HavokEntity * entity, hkLong phantomid );

	~BoostAction(void);

	/**
	 * Get the phantom ID
	 * @return phantom ID
	 */
	inline hkUlong& getPhantomId() {
		return mPhantomId;
	}

private:

	virtual void applyAction( const hkStepInfo& stepInfo );

	virtual hkpAction* clone( const hkArray<hkpEntity*>& newEntities, const hkArray<hkpPhantom*>& newPhantoms ) const { 
		return HK_NULL; 
	}
};

}

#endif
