#ifndef GAMEEVENTPARSER_H_
#define GAMEEVENTPARSER_H_

#include "NetworkEventParser.h"
#include "GameEvent.h"
#include "GameEventType.h"

namespace HovUni {

/**
 * A parser for parsing the controller events
 *
 * @author Olivier Berghmans
 */
class GameEventParser: public NetworkEventParser<GameEventType, GameEventTypeSize,3> {
public:
	/**
	 * Destructor
	 */
	~GameEventParser();

	/**
	 * @see NetworkEventParser::parse(ZCom_BitStream* stream)
	 */
	virtual GameEvent* parse(ZCom_BitStream* stream);
};

}

#endif
