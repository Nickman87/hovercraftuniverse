#ifndef FINISHPHANTOM_H
#define FINISHPHANTOM_H

#include <Physics/Dynamics/Phantom/hkpAabbPhantom.h>

namespace HovUni {

class Finish;

/**
 * A phantom used to check if a player passes the finish
 * @author Pieter-Jan Pintens
 */
class FinishPhantom : public hkpAabbPhantom
{
private:

	/**
	 * The finish object
	 */
	Finish * mFinish;

public:

	/**
	 * Constructor
	 * @param aabb, the bounding box
	 */
	FinishPhantom(const hkAabb& aabb, Finish * finish);

	~FinishPhantom(void);

	virtual void addOverlappingCollidable( hkpCollidable* handle );

	virtual void removeOverlappingCollidable( hkpCollidable* handle );
};

}

#endif