#ifndef SpeedBoostPhantom_H
#define SpeedBoostPhantom_H

#include <Physics/Dynamics/Phantom/hkpSimpleShapePhantom.h>

namespace HovUni {

class SpeedBoost;

/**
 * This is a boost phantom
 * When a player passes trough it should receive a boost or a slowdown.
 * @author Pieter-Jan Pintens
 */
class SpeedBoostPhantom : public hkpSimpleShapePhantom
{
private:

	SpeedBoost * mBoost;

public:

	/**
	 * Constructor 
	 * @param aabb, the bounding box
	 * @param boost
	 */
	SpeedBoostPhantom(const hkpShape *shape, const hkTransform &transform, SpeedBoost * boost);

	~SpeedBoostPhantom(void);

	virtual void addOverlappingCollidable( hkpCollidable* handle );

	virtual void removeOverlappingCollidable( hkpCollidable* handle );
};

}

#endif