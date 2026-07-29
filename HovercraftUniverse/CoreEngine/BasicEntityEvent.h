#ifndef BASICENTITYEVENT_H_
#define BASICENTITYEVENT_H_

#include "ControllerEvent.h"
#include <OgreVector3.h>

namespace HovUni {

/**
 * This is a basic entity event which involves changing position
 * and orientation.
 *
 * @author Olivier Berghmans
 */
class BasicEntityEvent: public ControllerEvent
{
private:
	/** Whether the entity should move forward */
	bool mForward;

	/** Whether the entity should move backward */
	bool mBackward;

	/** Whether the entity should move left */
	bool mLeft;

	/** Whether the entity should move right */
	bool mRight;

	/** Whether the entity should be reseted */
	bool mReset;

public:

	/**
	 * Default constructor for parsing
	 */
	BasicEntityEvent();

	/**
	 * Constructor
	 *
	 * @param forward whether the entity should move forward
	 * @param backward whether the entity should move backward
	 * @param left whether the entity should move left
	 * @param right whether the entity should move right
	 * @param reset whether the entity should be reseted
	 */
	BasicEntityEvent(bool forward, bool backward, bool left, bool right, bool reset);

	/**
	 * Destructor
	 */
	~BasicEntityEvent();

	/**
	 * The comparison operator
	 *
	 * @return whether the two events are equal
	 */
	bool operator==(const BasicEntityEvent& event);

	/**
	 * Check whether the entity should move forward
	 *
	 * @return true if the entity should move forward
	 */
	bool moveForward() const;

	/**
	 * Check whether the entity should move backward
	 *
	 * @return true if the entity should move backward
	 */
	bool moveBackward() const;

	/**
	 * Check whether the entity should move left
	 *
	 * @return true if the entity should move left
	 */
	bool moveLeft() const;

	/**
	 * Check whether the entity should move right
	 *
	 * @return true if the entity should move right
	 */
	bool moveRight() const;

	/**
	 * Check whether the entity should be reseted
	 *
	 * @return true if the entity should be reseted
	 */
	bool reset() const;

	/**
	 * Parse the stream to create this event
	 *
	 * @param stream the bitstream containing the event
	 * @return the event
	 */
	static BasicEntityEvent* parse(ZCom_BitStream* stream);

	/**
	 * Clears the event
	 */
	void clear();

protected:

	/**
	 * @see ControllerEvent::write(ZCom_BitStream* stream)
	 */
	virtual void write(ZCom_BitStream* stream) const;

	/**
	 * @see ControllerEvent::read(ZCom_BitStream* stream)
	 */
	virtual void read(ZCom_BitStream* stream);

};

}

#endif
