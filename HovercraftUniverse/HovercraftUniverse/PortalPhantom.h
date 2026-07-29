#ifndef PORTALPHANTOM_H
#define PORTALPHANTOM_H

#include <Physics/Dynamics/Phantom/hkpAabbPhantom.h>

namespace HovUni {

class Portal;

/**
 * A phantom to check if a player entered a teleport portal
 * @author Pieter-Jan Pintens
 */
class PortalPhantom : public hkpAabbPhantom
{
private:

	Portal * mPortal;

public:

	/**
	 * Constructor
	 * @param bounding box
	 * @param portal
	 */
	PortalPhantom(const hkAabb& aabb, Portal * portal);

	~PortalPhantom(void);

	virtual void addOverlappingCollidable( hkpCollidable* handle );

	virtual void removeOverlappingCollidable( hkpCollidable* handle );
};

}

#endif