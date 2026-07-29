#ifndef NOTIFYEVENT_H_
#define NOTIFYEVENT_H_

#include "ChatEvent.h"
#include <string>

namespace HovUni {

/**
 * This is a notify event which contains a notification of the server
 *
 * @author Olivier Berghmans
 */
class NotifyEvent: public ChatEvent {
private:

public:
	/**
	 * Constructor
	 *
	 * @param notif the notification
	 */
	NotifyEvent(const std::string& notif);

	/**
	 * Destructor
	 */
	~NotifyEvent();

	/**
	 * Parse the stream to create this event
	 *
	 * @param stream the bitstream containing the event
	 * @return the event
	 */
	static NotifyEvent* parse(ZCom_BitStream* stream);

protected:
	/**
	 * Default constructor for parsing
	 */
	NotifyEvent();

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
