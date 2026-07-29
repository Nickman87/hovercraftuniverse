#ifndef CHATSERVER_H_
#define CHATSERVER_H_

#include "NetworkServer.h"
#include "NetworkIDManager.h"
#include "ChatEntity.h"

namespace HovUni {

/**
 * This class represents the core of the server. It holds the controls
 * for the networking etc.
 *
 * @author Olivier Berghmans
 */
class ChatServer: public NetworkServer {
private:
	/** The chat entity **/
	ChatEntity* mChat;

	/** The ID manager */
	NetworkIDManager* mIDManager;

public:
	/**
	 * Constructor
	 */
	ChatServer();

	/**
	 * Destructor
	 */
	~ChatServer();

	/**
	 * Process incoming and outgoing packets
	 */
	virtual void process();

	/**
	 * Send a notification
	 *
	 * @param notification the notification
	 */
	void sendNotification(const std::string& notification);

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
	void ZCom_cbConnectionClosed(ZCom_ConnID id, eZCom_CloseReason reason,
			ZCom_BitStream& reasondata);

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
	 * @return true to accept the connection, false otherwise.
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
	void ZCom_cbZoidResult(ZCom_ConnID id, eZCom_ZoidResult result, zU8 new_level,
			ZCom_BitStream& reason);
};

}

#endif
