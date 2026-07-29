#ifndef CHATEVENT_H_
#define CHATEVENT_H_

#include "NetworkEvent.h"
#include "ChatEventType.h"
#include <string>

namespace HovUni {

/**
 * This is an event during the game. It is serializable so that
 * it can be sent across the network.
 *
 * @author Olivier Berghmans
 */
class ChatEvent: public NetworkEvent<ChatEventType, ChatEventTypeSize,1> {
protected:
	/** The chat line */
	std::string mLine;

public:
	/**
	 * Constructor
	 *
	 * @param type the type of the event (used for serialization)
	 * @param line the chat line
	 */
	ChatEvent(ChatEventType type, const std::string& line);

	/**
	 * Destructor
	 */
	~ChatEvent();

	/**
	 * Get the chat line
	 *
	 * @return the line
	 */
	std::string getLine() const;
};

}

#endif
