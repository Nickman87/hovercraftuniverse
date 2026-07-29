#ifndef COLLISIONEVENT_H
#define COLLISIONEVENT_H

#include <OgreVector3.h>
#include "VisualEvent.h"

namespace HovUni {

class CollisionEvent :	public VisualEvent
{
private:

	/** Posistion of the collision */
	Ogre::Vector3 mPosition;

	/** Direction oposite to the hit */
	Ogre::Vector3 mNormal;

public:

	/**
	 * Constructor
	 * @param position
	 * @param normal
	 */
	CollisionEvent(const Ogre::Vector3& position, const Ogre::Vector3& normal);

	CollisionEvent(const CollisionEvent&);

	/**
	 * Constructor
	 */
	CollisionEvent();

	~CollisionEvent(void);

	/**
	 * Get the position of the collision
	 * @return the position of the collision
	 */
	const Ogre::Vector3& getPosition() const {
		return mPosition;
	}

	/**
	 * Get the normal of the collision
	 * @return the normal of the collision
	 */
	const Ogre::Vector3& getNormal() const {
		return mNormal;
	}

	//Network functionality

	/**
	 * Write the event to a bitstream
	 *
	 * @param stream the stream to write to
	 */
	void write(ZCom_BitStream* stream) const;

	/**
	 * Read the event from a bitstream
	 *
	 * @param stream the stream to read from
	 */
	void read(ZCom_BitStream* stream);

	/**
	 * Parse the event from a stream
	 *
	 * @param stream the stream to parse from
	 * @param the event
	 */
	static CollisionEvent* parse(ZCom_BitStream* stream);
};

}

#endif