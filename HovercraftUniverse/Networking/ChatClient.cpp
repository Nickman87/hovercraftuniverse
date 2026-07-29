#include "ChatClient.h"
#include "Exception.h"

namespace HovUni {

ChatClient::ChatClient(const std::string& username, const char* name, unsigned int port) :
	NetworkClient(name, port, "ChatClient"), mChat(0), mIDManager(0), mUser(username) {
	initialize();
}

ChatClient::ChatClient(const std::string& username) :
	NetworkClient(2378), mChat(0), mIDManager(0), mUser(username) {
	initialize();
}

ChatClient::~ChatClient() {

}

void ChatClient::initialize() {
	mIDManager = new NetworkIDManager(this);
	mIDManager->registerClass(ChatEntity::getClassName());
	ZCom_setUpstreamLimit(0, 0);
}

void ChatClient::addListener(ChatListener* listener) {
	if (mChat) {
		mChat->addListener(listener);
	} else {
		Listenable<ChatListener>::addListener(listener);
	}
}

void ChatClient::removeListener(ChatListener* listener) {
	if (mChat) {
		mChat->removeListener(listener);
	} else {
		Listenable<ChatListener>::removeListener(listener);
	}
}

void ChatClient::process() {
	NetworkClient::process(1);
	if (mChat) {
		mChat->processEvents(0.0f);
	}
}

void ChatClient::setUsername(const std::string& username) {
	mUser = username;
}

void ChatClient::sendText(const std::string& line) {
	if (mChat) {
		mChat->sendLine(mUser, line);
	}
}

void ChatClient::onConnectResult(eZCom_ConnectResult result, ZCom_BitStream& extra) {
	if (result == eZCom_ConnAccepted) {
		// Connection accepted, so request zoid level
		ZCom_requestZoidMode(mConnID, 1);
	} else {
		// Connection failed
		THROW(NetworkException, "Connection failed");
		return;
	}
}

void ChatClient::onDisconnect(eZCom_CloseReason reason, ZCom_BitStream& extra) {
	// Connection closed
}

void ChatClient::onDataReceived(ZCom_BitStream& data) {
	// Data received
}

void ChatClient::onZoidResult(eZCom_ZoidResult result, zU8 new_level, ZCom_BitStream& reason) {
	if (result == eZCom_ZoidEnabled) {
		// Requested zoid level was confirmed
	} else {
		// Requested zoid level denied
	}
}

void ChatClient::onNodeDynamic(ZCom_ClassID requested_class, ZCom_BitStream* announcedata,
		eZCom_NodeRole role, ZCom_NodeID net_id) {
	if (requested_class == mIDManager->getID(ChatEntity::getClassName())) {
		// Lobby received (upon connect)
		mChat = new ChatEntity();
		mChat->networkRegister(requested_class, this);

		for (listener_iterator it = listenersBegin(); it != listenersEnd(); ++it) {
			mChat->addListener(*it);
		}
		listenersClear();
	}
}

}
