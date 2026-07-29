#ifndef SAMPLEEVENTPARSER_H_
#define SAMPLEEVENTPARSER_H_

#include "NetworkEventParser.h"
#include "NetworkEvent.h"
#include "SampleEventType.h"

namespace HovUni {

class SampleEventParser: public NetworkEventParser<SampleEventType>
{
public:
	/**
	 * Destructor
	 */
	virtual ~SampleEventParser();

	/**
	 * Parse a stream to the correct event
	 *
	 * @param stream the bitstream containing the event
	 * @return the event
	 */
	virtual NetworkEvent<SampleEventType>* parse(ZCom_BitStream* stream);
};

}

#endif
