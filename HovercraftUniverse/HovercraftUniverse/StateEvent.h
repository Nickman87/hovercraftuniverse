#ifndef STATEEVENT_H
#define STATEEVENT_H

#include "GameEvent.h"

namespace HovUni {

/**
 * An event that can be sent on a state change
 *
 * @author Olivier Berghmans
 */
class StateEvent: public GameEvent {
private:
	// The state
	unsigned int mState;

public:

	/**
	 * Constructor
	 *
	 * @param state the state
	 */
	StateEvent(unsigned int state);

	/**
	 * Default constructor for event construction
	 */
	StateEvent();

	/**
	 * Destructor
	 */
	virtual ~StateEvent();

	/**
	 * Get the stream
	 *
	 * @return the stream
	 */
	unsigned int getState() const {
		return mState;
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
	static StateEvent* parse(ZCom_BitStream* stream);
};

}

#endif
