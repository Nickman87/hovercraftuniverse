#ifndef STARTPHANTOM_H
#define STARTPHANTOM_H

#include <Physics/Dynamics/Phantom/hkpAabbPhantom.h>

namespace HovUni {

class Start;

/**
 * A phantom to check if a player has passed the start line
 * @author Pieter-Jan Pintens
 */
class StartPhantom : public hkpAabbPhantom
{
private:

	Start * mStart;

public:
	/**
	 * Constructor
	 * @param aabb, the bounding box
	 */
	StartPhantom(const hkAabb& aabb, Start * start );

	~StartPhantom(void);

	virtual void addOverlappingCollidable( hkpCollidable* handle );

	virtual void removeOverlappingCollidable( hkpCollidable* handle );
};

}

#endif