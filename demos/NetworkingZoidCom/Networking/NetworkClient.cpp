#include "NetworkClient.h"
#include <sstream>

namespace HovUni {

NetworkClient::NetworkClient(const char* name, const unsigned port) : mServerName(name), mConnectPort(port) {
	initialize(true);
}

NetworkClient::NetworkClient(const unsigned port) : mServerName(0), mConnectPort(port) {
	initialize(false);
}


NetworkClient::~NetworkClient() {

}

void NetworkClient::initialize(bool remote) {
	ZCom_setDebugName("NetworkClient");

	// Create and initialize network sockets (UDP, UDP port, internal socket port)
	bool result = ZCom_initSockets(true, 0, (remote ? 0 : 1));

	if (!result) {
		// TODO Throw exception, cannot init sockets
	}

	// New scope to assure server_addr gets out of scope before zcom could be deleted
	{
		// Construct server address
		ZCom_Address server_addr;
		if (remote) {
			std::stringstream ss;
			ss << mServerName << ":" << mConnectPort;
			server_addr.setAddress(eZCom_AddressUDP, 0, ss.str().c_str());
		} else {
			server_addr.setType(eZCom_AddressLocal);
			server_addr.setPort(mConnectPort);
		}
		// Connect
		ZCom_ConnID connection_id = ZCom_Connect(server_addr, 0);
	}
}

void NetworkClient::process() {
	ZCom_processInput();
	ZCom_processOutput();
}

void NetworkClient::ZCom_cbConnectResult(ZCom_ConnID id, eZCom_ConnectResult result, ZCom_BitStream& reply) {

}

void NetworkClient::ZCom_cbConnectionClosed(ZCom_ConnID id, eZCom_CloseReason reason, ZCom_BitStream& reasondata) {

}

void NetworkClient::ZCom_cbDataReceived(ZCom_ConnID id, ZCom_BitStream& data) {

}  

void NetworkClient::ZCom_cbZoidResult(ZCom_ConnID id, eZCom_ZoidResult result, zU8 new_level, ZCom_BitStream& reason) {

}

void NetworkClient::ZCom_cbNodeRequest_Dynamic(ZCom_ConnID id, ZCom_ClassID requested_class, ZCom_BitStream* announcedata, eZCom_NodeRole role, ZCom_NodeID net_id) {

}

void NetworkClient::ZCom_cbNodeRequest_Tag(ZCom_ConnID id, ZCom_ClassID requested_class, ZCom_BitStream* announcedata, eZCom_NodeRole role, zU32 tag) {

}

bool NetworkClient::ZCom_cbDiscoverRequest(const ZCom_Address& addr, ZCom_BitStream& request, ZCom_BitStream& reply) {
	return false;
}

void NetworkClient::ZCom_cbDiscovered(const ZCom_Address& addr, ZCom_BitStream& reply) {

}

//
// Server only callbacks
//
bool NetworkClient::ZCom_cbConnectionRequest(ZCom_ConnID id, ZCom_BitStream& request, ZCom_BitStream& reply) {
	return false;
}

void NetworkClient::ZCom_cbConnectionSpawned(ZCom_ConnID id) {

}

bool NetworkClient::ZCom_cbZoidRequest(ZCom_ConnID id, zU8 requested_level, ZCom_BitStream& reason) {
	return false;
}

}