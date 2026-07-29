#ifndef PLANETGRAVITYPHANTOM_H
#define PLANETGRAVITYPHANTOM_H

#include <Physics/Dynamics/Phantom/hkpAabbPhantom.h>

namespace HovUni {

class Asteroid;

class PlanetGravityPhantom : public hkpAabbPhantom
{
private:

	/**
	 * The asteroid
	 */
	Asteroid * mAsteroid;
	
	/**
	 * Rigid body representing the planet
	 */
	hkpRigidBody* mPlanetBody;

public:

	/**
	 * Constructor
	 * @param asteroid
	 * @param the planet rb
	 * @param aabb
	 */
	PlanetGravityPhantom(Asteroid * asteroid, hkpRigidBody* body, const hkAabb& aabb);

	virtual ~PlanetGravityPhantom(void);

	virtual void addOverlappingCollidable( hkpCollidable* handle );

	virtual void removeOverlappingCollidable( hkpCollidable* handle );
};

}

#endif
