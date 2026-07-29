#ifndef NETWORKSERVER_H_
#define NETWORKSERVER_H_

#include <zoidcom/zoidcom.h>

namespace HovUni {

/**
 * This class represents a basic server without any logic. You should inherit from this
 * Server and reimplement the public callbacks to insert logic.
 *
 * @author Olivier Berghmans
 */
class NetworkServer : public ZCom_Control 
{
public:
	/**
	 * Constructor
	 *
	 * @param port the listening port of the server
	 * @param internalport the port the internal socket will be bound to, or 0 if socket won't be used. 
	 */
	NetworkServer(const unsigned port, const unsigned internalport);

	/**
	 * Destructor
	 */
	virtual ~NetworkServer();

	/**
	 * Process incoming and outgoing packets
	 */
	virtual void process();

private:
	/** The listen port */
	const unsigned mServerPort;

	/** The internal port */
	const unsigned mInternalPort;

//
// ZCom_Control callbacks
//
public:
	/**
	 * Connection has been closed and is about to be deleted (Server, Client).
	 *
	 * @param id The connection's ID
	 * @param reason Reason code. If reason is eZCom_ClosedDisconnect, then reasondata might contain more info.
	 * @param reasondata The reason of the disconnector.
	 */
	void ZCom_cbConnectionClosed(ZCom_ConnID id, eZCom_CloseReason reason, ZCom_BitStream& reasondata);

	/**
	 * Direct data has been received (Server, Client).
	 *
	 * @param id The connection's ID
	 * @param data The data received
	 */
	void ZCom_cbDataReceived(ZCom_ConnID id, ZCom_BitStream& data);

	/**
	 * Incoming connection (Server).
	 *
	 * @param id The connection's ID
	 * @param request The request given by the requester
	 * @param reply Data you want to transmit to the requester additionally to the yes/no reply
	 * @param true to accept the connection, false otherwise.
	 */
	bool ZCom_cbConnectionRequest(ZCom_ConnID id, ZCom_BitStream& request, ZCom_BitStream& reply);

	/**
	 * A granted, incoming connection has been fully set up (Server).
	 *
	 * @param id The connection's ID
	 */
	void ZCom_cbConnectionSpawned(ZCom_ConnID id);

	/**
	 * A client requests to enter a specified ZoidLevel (Server).
	 *
	 * @param id The connection's ID
	 * @param requested_level Level the client wants to enter
	 * @param reason When returning false, this is returned as the reason why.
	 * @return True to accept the request, false otherwise.
	 */
	bool ZCom_cbZoidRequest(ZCom_ConnID id, zU8 requested_level, ZCom_BitStream& reason);

	/**
	 * ZoidLevel migration process finished (Server, Client).
	 *
	 * @param id The connection's ID
	 * @param result Result of the migration process
	 * @param new_level New ZoidLevel of the connection
	 * @param reason Additional data passed by the server
	 */
	void ZCom_cbZoidResult(ZCom_ConnID id, eZCom_ZoidResult result, zU8 new_level, ZCom_BitStream& reason);

	/**
	 * Another ZCom_Control has sent a discover request (Server, Client).
	 *
	 * @param addr The address where the request came from
	 * @param request Additional application provided data to the request
	 * @param reply Additional data the application wants to transmit to the discoverer
	 * @return True if you want to reply, false otherwise.
	 */
	bool ZCom_cbDiscoverRequest(const ZCom_Address& addr, ZCom_BitStream& request, ZCom_BitStream& reply);

	/**
	 * Another ZCom_Control responded to our discover request (Server, Client).
	 *
	 * @param addr The address where the discovered ZCom_Control is reachable
	 * @param reply Additional data sent back
	 */
	void ZCom_cbDiscovered(const ZCom_Address& addr, ZCom_BitStream& reply);
	
private:
	/**
	 * Connection process finished (Client).
	 *
	 * @param id The connection's ID
	 * @param result The result of the connection process
	 * @param reply The reply given by the destination
	 */
	void ZCom_cbConnectResult(ZCom_ConnID id, eZCom_ConnectResult result, ZCom_BitStream& reply);

	/**
	 * Server requests to create a new node for a new dynamic object/node (Client).
	 *
	 * @param id The connection's ID
	 * @param requested_class Class ID of the requested node
	 * @param announcedata Data set by the server
	 * @param role Role the newly created node will posses
	 * @param net_id The network ID of the requested node. Not needed directly here but may be useful.
	 */
	void ZCom_cbNodeRequest_Dynamic(ZCom_ConnID id, ZCom_ClassID requested_class, ZCom_BitStream* announcedata, eZCom_NodeRole role, ZCom_NodeID net_id);

	/**
	 * Server requests to create a new local node for a remote tagnode (Client).
	 *
	 * @param id The connection's ID
	 * @param requested_class Class ID of the requested node
	 * @param announcedata Data set by the server
	 * @param role Role the newly created node will posses
	 * @param tag The tag that was specified when the node was created on server
	 */
	void ZCom_cbNodeRequest_Tag(ZCom_ConnID id, ZCom_ClassID requested_class, ZCom_BitStream* announcedata, eZCom_NodeRole role, zU32 tag);

};

}

#endif
