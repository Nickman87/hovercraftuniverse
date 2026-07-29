#include "Exception.h"
#include "NetworkClient.h"
#include <sstream>

namespace HovUni {

NetworkClient::NetworkClient(const char* name, const unsigned port, const char* debugname) :
	mServerName(name), mConnectPort(port), mDebugName(debugname), mRemote(true), mConnID(
			ZCom_Invalid_ID), mConnected(false) {
	initialize();
}

NetworkClient::NetworkClient(const unsigned port, const char* debugname) :
	mServerName(0), mConnectPort(port), mDebugName(debugname), mRemote(false), mConnID(
			ZCom_Invalid_ID), mConnected(false) {
	initialize();
}

NetworkClient::~NetworkClient() {
}

void NetworkClient::initialize() {
	ZCom_setDebugName(mDebugName);

	// Create and initialize network sockets (UDP, UDP port, internal socket port)
	bool result = ZCom_initSockets(true, 0, (mRemote ? 0 : 1));

	if (!result) {
		THROW(NetworkException, "Cannot initialize sockets");
	}
}

void NetworkClient::connect(ZCom_BitStream * request) {
	// New scope to assure server_addr gets out of scope before zcom could be deleted
	{
		// Construct server address
		ZCom_Address server_addr;
		if (mRemote) {
			std::stringstream ss;
			ss << mServerName << ":" << mConnectPort;
			server_addr.setAddress(eZCom_AddressUDP, 0, ss.str().c_str());
		} else {
			server_addr.setType(eZCom_AddressLocal);
			server_addr.setPort(mConnectPort);
		}
		// Connect
		mConnID = ZCom_Connect(server_addr, request);

		if (mConnID == ZCom_Invalid_ID) {
			THROW(NetworkException, "Connection failed");
		}
	}
}

void NetworkClient::disconnect(const std::string& reason) {
	if (mConnected) {
		ZCom_BitStream* stream = new ZCom_BitStream();
		stream->addString(reason.c_str());
		ZCom_Disconnect(mConnID, stream);

		while (mConnected) {
			process(1);
		}
	}
}

void NetworkClient::process(zU32 simulationTimePassed) {
	//std::cout << "Processing replicator : " << simulationTimePassed << std::endl;
	ZCom_processReplicators(simulationTimePassed);
	ZCom_processInput();
	ZCom_processOutput();
	ZoidCom::Sleep(10);
}

void NetworkClient::onConnectResult(eZCom_ConnectResult result, ZCom_BitStream& extra) {
}

void NetworkClient::onDisconnect(eZCom_CloseReason reason, ZCom_BitStream& extra) {
}

void NetworkClient::onDataReceived(ZCom_BitStream& data) {
}

void NetworkClient::onZoidResult(eZCom_ZoidResult result, zU8 new_level, ZCom_BitStream& reason) {
}

void NetworkClient::onNodeDynamic(ZCom_ClassID requested_class, ZCom_BitStream* announcedata,
		eZCom_NodeRole role, ZCom_NodeID net_id) {
}

void NetworkClient::onNodeTag(ZCom_ClassID requested_class, ZCom_BitStream* announcedata,
		eZCom_NodeRole role, zU32 tag) {
}

bool NetworkClient::onDiscoverRequest(const ZCom_Address& addr, ZCom_BitStream& request,
		ZCom_BitStream& reply) {
	return false;
}

void NetworkClient::onDiscovered(const ZCom_Address& addr, ZCom_BitStream& reply) {
}

void NetworkClient::ZCom_cbConnectResult(ZCom_ConnID id, eZCom_ConnectResult result,
		ZCom_BitStream& reply) {
	if (result == eZCom_ConnAccepted) {
		//Mark connected
		mConnected = true;
	} else {
		//Mark not connected
		mConnected = false;
	}

	onConnectResult(result, reply);
}

void NetworkClient::ZCom_cbConnectionClosed(ZCom_ConnID id, eZCom_CloseReason reason,
		ZCom_BitStream& reasondata) {
	mConnected = false;
	mConnID = ZCom_Invalid_ID;
	onDisconnect(reason, reasondata);
}

void NetworkClient::ZCom_cbDataReceived(ZCom_ConnID id, ZCom_BitStream& data) {
	onDataReceived(data);
}

void NetworkClient::ZCom_cbZoidResult(ZCom_ConnID id, eZCom_ZoidResult result, zU8 new_level,
		ZCom_BitStream& reason) {
	onZoidResult(result, new_level, reason);
}

void NetworkClient::ZCom_cbNodeRequest_Dynamic(ZCom_ConnID id, ZCom_ClassID requested_class,
		ZCom_BitStream* announcedata, eZCom_NodeRole role, ZCom_NodeID net_id) {
	onNodeDynamic(requested_class, announcedata, role, net_id);
}

void NetworkClient::ZCom_cbNodeRequest_Tag(ZCom_ConnID id, ZCom_ClassID requested_class,
		ZCom_BitStream* announcedata, eZCom_NodeRole role, zU32 tag) {
	onNodeTag(requested_class, announcedata, role, tag);
}

bool NetworkClient::ZCom_cbDiscoverRequest(const ZCom_Address& addr, ZCom_BitStream& request,
		ZCom_BitStream& reply) {
	return onDiscoverRequest(addr, request, reply);
}

void NetworkClient::ZCom_cbDiscovered(const ZCom_Address& addr, ZCom_BitStream& reply) {
	onDiscovered(addr, reply);
}

//
// Server only callbacks
//
bool NetworkClient::ZCom_cbConnectionRequest(ZCom_ConnID id, ZCom_BitStream& request,
		ZCom_BitStream& reply) {
	return false;
}

void NetworkClient::ZCom_cbConnectionSpawned(ZCom_ConnID id) {

}

bool NetworkClient::ZCom_cbZoidRequest(ZCom_ConnID id, zU8 requested_level, ZCom_BitStream& reason) {
	return false;
}

}
