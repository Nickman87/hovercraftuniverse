#include "ChatServer.h"

namespace HovUni {

ChatServer::ChatServer() :
	NetworkServer(2377, 2378, "ChatServer"), mChat(0), mIDManager(0) {
	// Create and store entity manager
	mIDManager = new NetworkIDManager(this);
	mIDManager->registerClass(ChatEntity::getClassName());
	ZCom_setUpstreamLimit(0, 0);

	// Create chat entity
	mChat = new ChatEntity();

	// Register chat entity
	mChat->networkRegister(mIDManager->getID(ChatEntity::getClassName()), this);
}

ChatServer::~ChatServer() {
	delete mChat;
	mChat = 0;
}

void ChatServer::process() {
	NetworkServer::process(1);
	mChat->processEvents(0.0f);
}

void ChatServer::sendNotification(const std::string& notification) {
	if (mChat) {
		mChat->sendNotification(notification);
	}
}

bool ChatServer::ZCom_cbConnectionRequest(ZCom_ConnID id, ZCom_BitStream& request,
		ZCom_BitStream& reply) {
	return true;
}

void ChatServer::ZCom_cbConnectionSpawned(ZCom_ConnID id) {
	mChat->getNetworkNode()->setOwner(id, true);
}

void ChatServer::ZCom_cbConnectionClosed(ZCom_ConnID id, eZCom_CloseReason reason,
		ZCom_BitStream& reasondata) {
	mChat->getNetworkNode()->setOwner(id, false);
	if (reason == eZCom_ClosedDisconnect) {
		std::string str = reasondata.getString();
		sendNotification(str);
	}
}

void ChatServer::ZCom_cbDataReceived(ZCom_ConnID id, ZCom_BitStream& data) {

}

bool ChatServer::ZCom_cbZoidRequest(ZCom_ConnID id, zU8 requested_level, ZCom_BitStream& reason) {
	// Only accept level 1
	if (requested_level == 1) {
		return true;
	} else {
		return false;
	}
}

void ChatServer::ZCom_cbZoidResult(ZCom_ConnID id, eZCom_ZoidResult result, zU8 new_level,
		ZCom_BitStream& reason) {
	// Result of zoid request
}

}
