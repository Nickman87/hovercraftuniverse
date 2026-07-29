#ifndef TEXTEVENT_H_
#define TEXTEVENT_H_

#include "ChatEvent.h"
#include <string>

namespace HovUni {

/**
 * This is a chat event which contains a text line sent by a user
 *
 * @author Olivier Berghmans
 */
class TextEvent: public ChatEvent {
private:
	/** The user */
	std::string mUser;

public:
	/**
	 * Constructor
	 *
	 * @param user the user name sending the chat line
	 * @param line the chat line
	 */
	TextEvent(const std::string& user, const std::string& line);

	/**
	 * Destructor
	 */
	~TextEvent();

	/**
	 * Get the user name
	 *
	 * @return the name
	 */
	std::string getUser() const;

	/**
	 * Parse the stream to create this event
	 *
	 * @param stream the bitstream containing the event
	 * @return the event
	 */
	static TextEvent* parse(ZCom_BitStream* stream);

protected:
	/**
	 * Default constructor for parsing
	 */
	TextEvent();

	/**
	 * @see ChatEvent::write(ZCom_BitStream* stream)
	 */
	virtual void write(ZCom_BitStream* stream) const;

	/**
	 * @see ChatEvent::read(ZCom_BitStream* stream)
	 */
	virtual void read(ZCom_BitStream* stream);

};

}

#endif
