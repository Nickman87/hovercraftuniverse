#ifndef NETWORKEVENTPARSER_H_
#define NETWORKEVENTPARSER_H_

#include "NetworkEvent.h"

namespace HovUni {

/**
 * This is an interface for an event parser.
 *
 * @author Olivier Berghmans
 */
template<typename EventType, int N, int EventClass>
class NetworkEventParser {
public:
	/**
	 * The parser that will be used for this entity
	 */
	typedef EventType event_type;

	/**
	 * Destructor
	 */
	virtual ~NetworkEventParser() {
	}

	/**
	 * Parse a stream to the correct event
	 *
	 * @param stream the bitstream containing the event
	 * @return the event, 0 if not a right event
	 */
	virtual NetworkEvent<EventType, N, EventClass>* parse(ZCom_BitStream* stream) = 0;
};

}

#endif
