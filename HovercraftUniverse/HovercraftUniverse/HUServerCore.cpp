#include "HUServerCore.h"
#include "ServerLoader.h"
#include "EntityRegister.h"
#include "DedicatedServer.h"
#include "ChatServer.h"
#include "EntityMapping.h"

namespace HovUni {

HUServerCore::HUServerCore() : NetworkServer(2375, 2376, "HUServer"), mEntityManager(0), mIDManager(0) {
	// Create the chat server
	mChatServer = new ChatServer();

	// Create and store entity manager
	mEntityManager = EntityManager::getServerSingletonPtr();

	
	EntityMapping::setEntityIniFile(DedicatedServer::getConfig()->getValue<std::string>("Entities", "Path", "entities\\") +
		DedicatedServer::getConfig()->getValue<std::string>("Entities", "File", "Entities.ini"));

	mIDManager = NetworkIDManager::getServerSingletonPtr();
	mIDManager->setControl(this);
	EntityRegister::registerAll(*mIDManager);
	ZCom_setUpstreamLimit(0, 0);

	// Create lobby
	mLobby = new Lobby(new ServerLoader());
	mLobby->setChatServer(mChatServer);
	mLobby->getNetworkNode()->setEventNotification(true, false);
	// Register lobby
	mLobby->networkRegisterUnique(mIDManager->getID(Lobby::getClassName()), this, true);
}

HUServerCore::~HUServerCore() {
	delete mLobby;
	mLobby;
	delete mChatServer;
	mChatServer = 0;
}

void HUServerCore::process(zU32 simulationTimePassed) {
	NetworkServer::process(simulationTimePassed);
	mLobby->process();

	//Update the chat server
	mChatServer->process();
}

bool HUServerCore::ZCom_cbConnectionRequest(ZCom_ConnID id, ZCom_BitStream& request, ZCom_BitStream& reply) {
	// Accept a connection if lobby isn't full
	reply.addInt(id, sizeof(ZCom_ConnID) * 8);
	return mLobby->onConnectAttempt(id);
}

void HUServerCore::ZCom_cbConnectionSpawned(ZCom_ConnID id) {
	ZCom_requestDownstreamLimit(id, 60, 600);

	// Notice the lobby of new connection
	mLobby->onConnect(id);
}

void HUServerCore::ZCom_cbConnectionClosed(ZCom_ConnID id, eZCom_CloseReason reason, ZCom_BitStream& reasondata) {
	// Connection closed
	std::string reasonStr = "";
	if (reason == eZCom_ClosedDisconnect) {
		reasonStr = reasondata.getString();
	}
	mLobby->onDisconnect(id, reasonStr);
}

void HUServerCore::ZCom_cbDataReceived(ZCom_ConnID id, ZCom_BitStream& data) {

}

bool HUServerCore::ZCom_cbZoidRequest(ZCom_ConnID id, zU8 requested_level, ZCom_BitStream& reason) {
	// Only accept level 1
	if (requested_level == 1) {
		return true;
	} else {
		return false;
	}
}

void HUServerCore::ZCom_cbZoidResult(ZCom_ConnID id, eZCom_ZoidResult result, zU8 new_level, ZCom_BitStream& reason) {
	// Result of zoid request
}

}
