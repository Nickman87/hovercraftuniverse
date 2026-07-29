#ifndef HAVOKENTITY_H
#define HAVOKENTITY_H

#include <Common/Base/hkBase.h>

class hkpRigidBody;
class hkAabb;
class hkpWorld;

namespace HovUni {

class Entity;

/**
 * A character, this interface should be used as container for the physical representation of game entities.
 */
class HavokEntity {

protected:

	hkpWorld * mWorld;

public:

	static const hkVector4 UP;

	static const hkVector4 FORWARD;

	/**
	 * Constructor
	 * @param world
	 */
	HavokEntity( hkpWorld * world );

	/**
	 * Destructor
	 */
	virtual ~HavokEntity();

	/**
	 * Get the up of the character
	 * @return the up of the character
	 */
	virtual const hkVector4& getUp() const = 0;

	/**
	 * Get the position of the character
	 * @return the position of the character
	 */
	virtual const hkVector4& getPosition() const = 0;

	/**
	* Get the linear velocity of the character (Dirk, 24/03/2010)
	* @reurn the linear velocity of the character
	*/
	virtual const hkVector4& getVelocity() const = 0;

	/**
	 * Get the orientation of the character
	 * @return the orientation of the character
	 */
	virtual const hkQuaternion& getOrientation() const = 0;

	/**
	 * Get the rigid body that is used for the entity
	 * @return the rigid body used for the entity
	 */
	virtual hkpRigidBody * getRigidBody() const = 0;

	/**
	 * Get the entity
	 * @return entity
	 */
	virtual Entity * getEntity() = 0;

	/**
	 * Update the character based on movement
	 */
	virtual void update() = 0;

	/**
	 * Update the up vector
	 * @param up vector
	 */
	virtual void updateUp( const hkVector4& newUp) = 0;
	
};

}

#endif