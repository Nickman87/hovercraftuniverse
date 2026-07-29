#ifndef CHECKPOINTEVENT_H
#define CHECKPOINTEVENT_H

#include "GameEvent.h"

namespace HovUni {

/**
 * Event that indicates that a player has reached a checkpoint at a certain timestamp
 *
 * @author Olivier Berghmans
 */
class CheckpointEvent: public GameEvent {
private:
	/** The ID of the user */
	unsigned int mUser;

	/** The ID of the checkpoint */
	unsigned int mCheckpoint;

	/** The timestamp at the checkpoint */
	long mTimestamp;

public:
	/**
	 * Constructor
	 *
	 * @param user the ID of the user
	 * @param checkpoint the ID of the checkpoint
	 * @param time the timestamp at the checkpiont
	 */
	CheckpointEvent(unsigned int user, unsigned int checkpoint, long time);

	/**
	 * Default constructor for event construction
	 */
	CheckpointEvent();

	/**
	 * Destructor
	 */
	~CheckpointEvent();

	/**
	 * Get the ID of the user
	 *
	 * @return the ID
	 */
	unsigned int getUser() const {
		return mUser;
	}

	/**
	 * Get the ID of the checkpoint
	 *
	 * @return the ID
	 */
	unsigned int getCheckpoint() const {
		return mCheckpoint;
	}

	/**
	 * Get the timestamp at the checkpoint
	 *
	 * @return the timestamp
	 */
	long getTimestamp() const {
		return mTimestamp;
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
	static CheckpointEvent* parse(ZCom_BitStream* stream);
};

}

#endif
