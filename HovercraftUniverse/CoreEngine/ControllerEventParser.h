#ifndef CONTROLLEREVENTPARSER_H_
#define CONTROLLEREVENTPARSER_H_

#include "NetworkEventParser.h"
#include "ControllerEvent.h"
#include "ControllerEventType.h"

namespace HovUni {

/**
 * A parser for parsing the controller events
 *
 * @author Olivier Berghmans
 */
class ControllerEventParser: public NetworkEventParser<ControllerEventType, ControllerEventTypeSize,2>
{
public:
	/**
	 * Destructor
	 */
	~ControllerEventParser();

	/**
	 * @see NetworkEventParser::parse(ZCom_BitStream* stream)
	 */
	virtual ControllerEvent* parse(ZCom_BitStream* stream);
};

}

#endif
