#ifndef NETWORKEVENT_H_
#define NETWORKEVENT_H_

#include <zoidcom/zoidcom.h>

namespace HovUni {

/**
 * This is an event that occurs for an entity. It is used for sending over the
 * network so it should be serializable.
 *
 * @author Olivier Berghmans
 */
template <typename EventType>
class NetworkEvent
{
public:
	/**
	 * The parser that will be used for this entity
	 */
	typedef EventType event_type;

	/**
	 * Constructor
	 *
	 * @param type the type of the entity (used for serialization)
	 */
	NetworkEvent(EventType type);

	/**
	 * Destructor
	 */
	virtual ~NetworkEvent();

	/**
	 * Get the type of the event
	 *
	 * @return the type
	 */
	EventType getType() const;

	/**
	 * Serialize the event
	 *
	 * @param stream the bitstream to append the event
	 */
	void serialize(ZCom_BitStream* stream) const;

	/**
	 * Deserialize the event
	 *
	 * @param stream the bitstream containing the event
	 */
	void deserialize(ZCom_BitStream* stream);

	/**
	 * Write the event
	 *
	 * @param stream the bitstream to append the event
	 */
	virtual void write(ZCom_BitStream* stream) const = 0;

	/**
	 * Read the event
	 *
	 * @param stream the bitstream containing the event
	 */
	virtual void read(ZCom_BitStream* stream) = 0;

	/**
	 * Read the type of the event from the stream. However,
	 * this read just "peeks" at the type and does not change
	 * any state in the stream. In other words, after using
	 * this method, the deserialize method can still be used.
	 *
	 * @param stream the bitstream containing the event
	 */
	static EventType readType(ZCom_BitStream* stream);

private:
	/** The type of the event */
	EventType mType;
};

}

#include "NetworkEventImpl.h"

#endif
