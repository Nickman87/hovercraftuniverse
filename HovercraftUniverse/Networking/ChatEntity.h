#ifndef CHATENTITY_H
#define CHATENTITY_H

#include "NetworkEntity.h"
#include "ChatEvent.h"
#include "Listenable.h"
#include "ChatListener.h"
#include <string>
#include <vector>

namespace HovUni {

/**
 * The chat entity which holds the chat lines and allows to send lines to the chat
 *
 * @author Olivier Berghmans
 */
class ChatEntity: public NetworkEntity, public Listenable<ChatListener> {
public:

	/**
	 * Constructor
	 */
	ChatEntity();

	/**
	 * Destructor
	 */
	~ChatEntity();

	/**
	 * Send a chat line
	 *
	 * @param user the user sending the line
	 * @param line the chat line
	 */
	void sendLine(const string& user, const std::string& line);

	/**
	 * Send a chat line
	 *
	 * @param notif the notification
	 */
	void sendNotification(const std::string& notif);

	/**
	 * Get the class name for this class. This is used for registering
	 * the class with the network
	 *
	 * @return the class name
	 */
	static std::string getClassName();

private:
	/**
	 * @see NetworkEntity::parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id, ZCom_BitStream* stream, float timeSince);
	 */
	virtual void parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id,
			ZCom_BitStream* stream, float timeSince);

	/**
	 * @see NetworkEntity::setupReplication();
	 */
	virtual void setupReplication();

	/**
	 * @see NetworkEntity::setAnnouncementData(ZCom_BitStream* stream)
	 */
	void setAnnouncementData(ZCom_BitStream* stream);

	/**
	 * Process a chat event at the server
	 *
	 * @param event an event
	 */
	void processEventsServer(ChatEvent* cEvent);

	/**
	 * Process a chat event at the client
	 *
	 * @param event an event
	 */
	void processEventsClient(ChatEvent* cEvent);

};

}

#endif
