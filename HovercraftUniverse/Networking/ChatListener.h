#ifndef CHATLISTENER_H
#define CHATLISTENER_H

namespace HovUni {

/**
 * A chat listener. When this is registered with a chat entity, 
 * this object will be called upon incoming messages.
 *
 * @author Olivier Berghmans
 */
class ChatListener {
public:
	/**
	 * Destructor
	 */
	virtual ~ChatListener() {
	}

	/**
	 * An incoming text message
	 *
	 * @param user the username of the incoming message
	 * @param line the chat line
	 */
	virtual void newMessage(const std::string& user, const std::string& line) = 0;

	/**
	 * An incoming notification from the server
	 *
	 * @param notif the notification
	 */
	virtual void newNotification(const std::string& notif) = 0;
};

}

#endif
