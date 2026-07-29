#ifndef POWERUPPHANTOM_H
#define POWERUPPHANTOM_H

#include <Physics/Dynamics/Phantom/hkpAabbPhantom.h>

namespace HovUni {

class PowerupSpawn;

/**
 * A phantom to check if a player has picked up a powerup
 * @author Pieter-Jan Pintens
 */
class PowerupPhantom : public hkpAabbPhantom
{
private:

	PowerupSpawn * mPowerupSpawn;

public:

	/**
	 * Constructor
	 * @param bounding box
	 * @param powerupspawn
	 */
	PowerupPhantom(const hkAabb& aabb, PowerupSpawn * powerupspawn );

	~PowerupPhantom(void);

	virtual void addOverlappingCollidable( hkpCollidable* handle );

	virtual void removeOverlappingCollidable( hkpCollidable* handle );
};

}

#endif