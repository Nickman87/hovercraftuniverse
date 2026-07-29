#ifndef NETWORKEVENTPARSER_H_
#define NETWORKEVENTPARSER_H_

#include "NetworkEvent.h"

namespace HovUni {

/**
 * This is an interface for an event parser.
 *
 * @author Olivier Berghmans
 */
template <typename EventType>
class NetworkEventParser
{
public:
	/**
	 * The parser that will be used for this entity
	 */
	typedef EventType event_type;

	/**
	 * Destructor
	 */
	virtual ~NetworkEventParser() { }

	/**
	 * Parse a stream to the correct event
	 *
	 * @param stream the bitstream containing the event
	 * @return the event
	 */
	virtual NetworkEvent<EventType>* parse(ZCom_BitStream* stream) = 0;
};

}

#endif
