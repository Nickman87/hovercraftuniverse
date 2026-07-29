#ifndef STATICBODYENTITY_H
#define STATICBODYENTITY_H

#include "HavokEntity.h"

namespace HovUni {

class StaticBody;

class StaticBodyEntity : HavokEntity
{
private:

	hkVector4 mUp;

	hkpRigidBody * mBody;

	StaticBody * mStaticBody;

public:

	StaticBodyEntity(hkpWorld * world, hkpRigidBody * Body, StaticBody * StaticBody);

	~StaticBodyEntity(void);

	/**
	 * Get the up of the character
	 * @return the up of the character
	 */
	virtual const hkVector4& getUp() const;

	/**
	 * Get the position of the character
	 * @return the position of the character
	 */
	virtual const hkVector4& getPosition() const;

	/**
	* Get the linear velocity of the character (Dirk, 24/03/2010)
	* @reurn the linear velocity of the character
	*/
	virtual const hkVector4& getVelocity() const;

	/**
	 * Get the orientation of the character
	 * @return the orientation of the character
	 */
	virtual const hkQuaternion& getOrientation() const;

	/**
	 * Get the rigid body that is used for the entity
	 * @return the rigid body used for the entity
	 */
	virtual hkpRigidBody * getRigidBody() const;

	/**
	 * Get the entity
	 * @return entity
	 */
	virtual Entity * getEntity();

	/**
	 * Update the character based on movement
	 */
	virtual void update();

	/**
	 * Update the up vector
	 * @param up vector
	 */
	virtual void updateUp( const hkVector4& newUp);
};

}

#endif