#include "ChatEntity.h"
#include "NetworkIDManager.h"
#include "ChatEventParser.h"
#include "TextEvent.h"
#include "NotifyEvent.h"
#include <iostream>

namespace HovUni {

std::string ChatEntity::getClassName() {
	return "ChatEntity";
}

ChatEntity::ChatEntity() :
	NetworkEntity(0) {

}

ChatEntity::~ChatEntity() {

}

void ChatEntity::sendLine(const string& user, const string& line) {
	sendEvent(TextEvent(user, line));
}

void ChatEntity::sendNotification(const std::string& notif) {
	if (mNode->getRole() == eZCom_RoleAuthority) {
		sendEvent(NotifyEvent(notif));
	}
}

void ChatEntity::parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id,
		ZCom_BitStream* stream, float timeSince) {
	//if user event
	if (type == eZCom_EventUser) {
		ChatEventParser p;
		ChatEvent* cEvent = p.parse(stream);
		eZCom_NodeRole role = mNode->getRole();
		switch (role) {
		case eZCom_RoleAuthority:
			processEventsServer(cEvent);
			break;
		default:
			processEventsClient(cEvent);
			break;
		}
		delete cEvent;
	}
}

void ChatEntity::processEventsServer(ChatEvent* cEvent) {
	// A text line
	TextEvent* text = dynamic_cast<TextEvent*> (cEvent);
	if (text) {
		// TODO Maybe some advanced checking for bad language?
		sendEvent(*text);
	}

	// Notification events are not processed because they 
	// are sent by the server and should not pass this function
}

void ChatEntity::processEventsClient(ChatEvent* cEvent) {
	// A text line
	TextEvent* text = dynamic_cast<TextEvent*> (cEvent);
	if (text) {
		// Update all the listeners
		std::string user = text->getUser();
		std::string line = text->getLine();
		for (listener_iterator it = listenersBegin(); it != listenersEnd(); ++it) {
			(*it)->newMessage(user, line);
		}
	}

	// A notification
	NotifyEvent* notify = dynamic_cast<NotifyEvent*> (cEvent);
	if (notify) {
		// Update all the listeners
		std::string line = notify->getLine();
		for (listener_iterator it = listenersBegin(); it != listenersEnd(); ++it) {
			(*it)->newNotification(line);
		}
	}
}

void ChatEntity::setupReplication() {

}

void ChatEntity::setAnnouncementData(ZCom_BitStream* stream) {

}

}
