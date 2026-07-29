#ifndef CHECKPOINTPHANTOM_H
#define CHECKPOINTPHANTOM_H

#include <Physics/Dynamics/Phantom/hkpAabbPhantom.h>

namespace HovUni {

class CheckPoint;

/**
 * This is a phantom for a checkpoint. It can be used to detect if a player passes a checkpoint.
 * @author Pieter-Jan Pintens
 */
class CheckpointPhantom : public hkpAabbPhantom
{
private:

	CheckPoint * mCheckPoint;

public:
	/**
	 * Constructor
	 * @param aabb, the bounding box
	 * @param checkpoint
	 */
	CheckpointPhantom(const hkAabb& aabb, CheckPoint * checkpoint);

	~CheckpointPhantom(void);

	virtual void addOverlappingCollidable( hkpCollidable* handle );

	virtual void removeOverlappingCollidable( hkpCollidable* handle );
};

}

#endif