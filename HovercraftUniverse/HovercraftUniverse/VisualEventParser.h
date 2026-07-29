#ifndef VISUALEVENTPARSER_H
#define VISUALEVENTPARSER_H

#include "NetworkEventParser.h"
#include "VisualEvent.h"
#include "VisualEventType.h"

namespace HovUni {

/**
 * A parser for parsing the visual events
 *
 * @author Pieter-Jan Pintens
 */
class VisualEventParser : public NetworkEventParser<VisualEventType, VisualEventTypeSize,4>
{

public:
	/**
	 * Destructor
	 */
	~VisualEventParser(void);

	/**
	 * @see NetworkEventParser::parse(ZCom_BitStream* stream)
	 */
	virtual VisualEvent* parse(ZCom_BitStream* stream);
};

}

#endif
