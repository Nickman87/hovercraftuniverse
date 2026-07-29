#ifndef CHATEVENTPARSER_H_
#define CHATEVENTPARSER_H_

#include "NetworkEventParser.h"
#include "ChatEvent.h"
#include "ChatEventType.h"

namespace HovUni {

/**
 * A parser for parsing the chat events
 *
 * @author Olivier Berghmans
 */
class ChatEventParser: public NetworkEventParser<ChatEventType, ChatEventTypeSize,1> {
public:
	/**
	 * Destructor
	 */
	~ChatEventParser();

	/**
	 * @see NetworkEventParser::parse(ZCom_BitStream* stream)
	 */
	virtual ChatEvent* parse(ZCom_BitStream* stream);
};

}

#endif
