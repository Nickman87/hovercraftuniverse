#ifndef SAMPLEEVENTMOVERIGHT_H_
#define SAMPLEEVENTMOVERIGHT_H_

#include "NetworkEvent.h"
#include "SampleEventType.h"

namespace HovUni {

class SampleEventMoveRight: public NetworkEvent<SampleEventType>
{
public:
	/**
	 * Constructor
	 *
	 * @param distance the distance to move right
	 */
	SampleEventMoveRight(float distance);

	/**
	 * Constructor for parsing
	 */
	SampleEventMoveRight();

	/**
	 * Destructor
	 */
	virtual ~SampleEventMoveRight();

	/**
	 * Get the distance
	 *
	 * @return the distance
	 */
	float getDistance() const;

	/**
	 * Write the event
	 *
	 * @param stream the bitstream to append the event
	 */
	virtual void write(ZCom_BitStream* stream) const;

	/**
	 * Read the event
	 *
	 * @param stream the bitstream containing the event
	 */
	virtual void read(ZCom_BitStream* stream);

	/**
	 * Parse the stream to create this event
	 *
	 * @param stream the bitstream containing the event
	 * @return the event
	 */
	static SampleEventMoveRight* parse(ZCom_BitStream* stream);

private:
	/** The distance */
	float mDistance;
};

}

#endif
