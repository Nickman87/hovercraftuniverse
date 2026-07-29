#include "NetworkServer.h"

namespace HovUni {

NetworkServer::NetworkServer(const unsigned port, const unsigned internalport) : mServerPort(port), mInternalPort(internalport) {
	ZCom_setDebugName("NetworkServer");

	// Create and initialize network sockets (UDP, UDP port, internal socket port)
	bool result = ZCom_initSockets(true, mServerPort, mInternalPort);

	if (!result) {
		// TODO Throw exception, cannot init sockets
	}

}

NetworkServer::~NetworkServer() {

}

void NetworkServer::process() {
	ZCom_processInput();
	ZCom_processOutput();
}

bool NetworkServer::ZCom_cbConnectionRequest(ZCom_ConnID id, ZCom_BitStream& request, ZCom_BitStream& reply) {
	return true;
}

void NetworkServer::ZCom_cbConnectionSpawned(ZCom_ConnID id) {

}

void NetworkServer::ZCom_cbConnectionClosed(ZCom_ConnID id, eZCom_CloseReason reason, ZCom_BitStream& reasondata) {

}

void NetworkServer::ZCom_cbDataReceived(ZCom_ConnID id, ZCom_BitStream& data) {

}

bool NetworkServer::ZCom_cbZoidRequest(ZCom_ConnID id, zU8 requested_level, ZCom_BitStream& reason) {
	return false;
}

void NetworkServer::ZCom_cbZoidResult(ZCom_ConnID id, eZCom_ZoidResult result, zU8 new_level, ZCom_BitStream& reason) {

}

bool NetworkServer::ZCom_cbDiscoverRequest(const ZCom_Address& addr, ZCom_BitStream& request, ZCom_BitStream& reply) {
	return false;
}

void NetworkServer::ZCom_cbDiscovered(const ZCom_Address& addr, ZCom_BitStream& reply) {

}

//
// Client only callbacks
//
void NetworkServer::ZCom_cbConnectResult(ZCom_ConnID id, eZCom_ConnectResult result, ZCom_BitStream& reply) {

}

void NetworkServer::ZCom_cbNodeRequest_Dynamic(ZCom_ConnID id, ZCom_ClassID requested_class, ZCom_BitStream* announcedata, eZCom_NodeRole role, ZCom_NodeID net_id) {

}

void NetworkServer::ZCom_cbNodeRequest_Tag(ZCom_ConnID id, ZCom_ClassID requested_class, ZCom_BitStream* announcedata, eZCom_NodeRole role, zU32 tag) {

}

}
