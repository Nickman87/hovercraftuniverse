#ifndef BOOSTPROPERTY_H
#define BOOSTPROPERTY_H

#include "EntityPropertySystem.h"
#include <OgreVector3.h>

namespace HovUni {

/**
 * Boost property
 */
class BoostProperty: public EntityProperty
{
public:

	static EntityPropertyTag<BoostProperty,1> KEY;

private:	

	Ogre::Real mBoost;

	Ogre::Real mGain;

	Ogre::Vector3 mDirection;

public:

	BoostProperty( Ogre::Real boost, Ogre::Real gain, const Ogre::Vector3& direction );

	BoostProperty(int id);

	virtual ~BoostProperty(void);

	/**
	 * Get the direction
	 *
	 * @return direction
	 */
	const Ogre::Vector3& getDirection() const {
		return mDirection;
	}

	/**
	 * Get the boost
	 *
	 * @return boost
	 */
	Ogre::Real getBoost() const {
		return mBoost;
	}

	/**
	 * Set the boost
	 *
	 * @param boost
	 */
	void setBoost( Ogre::Real boost ){
		mBoost = boost;
		update();
	}

	/**
	 * Get the gain
	 *
	 * @return gain
	 */
	Ogre::Real getGain() const {
		return mGain;
	}

	/**
	 * Set the gain
	 *
	 * @param gain
	 */
	void setGain( Ogre::Real gain ){
		mGain = gain;
		update();
	}

	/**
	 * @see EntityProperty
	 */
	virtual void write(ZCom_BitStream& bitstream) const;

	/**
	 * @see EntityProperty
	 */
	virtual void read(ZCom_BitStream& bitstream);
};

}

#endif
