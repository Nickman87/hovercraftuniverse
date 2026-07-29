#ifndef CHATCLIENT_H_
#define CHATCLIENT_H_

#include "NetworkClient.h"
#include "NetworkIDManager.h"
#include "ChatEntity.h"
#include "Listenable.h"
#include "ChatListener.h"
#include <string>
#include <vector>

namespace HovUni {

/**
 * This class represents the core of the client. It holds the controls
 * for the networking etc.
 *
 * @author Olivier Berghmans
 */
class ChatClient: public NetworkClient, public Listenable<ChatListener> {
private:
	/** The chat entity **/
	ChatEntity* mChat;

	/** The ID manager */
	NetworkIDManager* mIDManager;

	/** The user name */
	std::string mUser;

public:
	/**
	 * Constructor for a client using a remote connection
	 *
	 * @param username the name of the user to chat
	 * @param name the name of the server
	 * @param port the port of the server
	 */
	ChatClient(const std::string& username, const char* name, unsigned int port = 2377);

	/**
	 * Constructor for a client using a local connection. This client
	 * must be running in the same instance of a program as the server. 
	 * It is used for combining the client and server to avoid the
	 * overhead of real network.
	 *
	 * @param username the name of the user to chat
	 */
	ChatClient(const std::string& username);

	/**
	 * Destructor
	 */
	~ChatClient();

	/**
	 * Add a listener
	 *
	 * @param listener the listener which will get called back upon events
	 */
	void addListener(ChatListener* listener);

	/**
	 * Remove a listener
	 *
	 * @param listener the listener
	 */
	void removeListener(ChatListener* listener);

	/**
	 * Process incoming and outgoing packets
	 */
	virtual void process();

	/**
	 * Set a new username
	 *
	 * @param username the username
	 */
	void setUsername(const std::string& username);

	/**
	 * Send a text line
	 *
	 * @param line the text line
	 */
	void sendText(const std::string& line);

private:
	/**
	 * Initialize the class
	 */
	void initialize();

	//
	// ZCom_Control callbacks
	//
protected:
	/**
	 * Callback for the connection result. This can be implemented in subclasses.
	 *
	 * @param result the result of the connection request
	 * @param extra information about the reply
	 */
	virtual void onConnectResult(eZCom_ConnectResult result, ZCom_BitStream& extra);

	/**
	 * Callback for the connection closure. This can be implemented in subclasses.
	 *
	 * @param reason the close reason
	 * @param extra more information about the reason
	 */
	virtual void onDisconnect(eZCom_CloseReason reason, ZCom_BitStream& extra);

	/**
	 * Callback for when data is received. This can be implemented in subclasses.
	 *
	 * @param data the incoming data
	 */
	virtual void onDataReceived(ZCom_BitStream& data);

	/**
	 * Callback for when the result of a zoid request is received. This can be implemented
	 * in subclasses.
	 *
	 * @param result the result of the request
	 * @param new_level the new zoid level
	 * @param reason the reason of this result
	 */
	virtual void onZoidResult(eZCom_ZoidResult result, zU8 new_level, ZCom_BitStream& reason);

	/**
	 * Callback for when the server requests for creating of a dynamic node. This must be
	 * implemented in subclasses.
	 *
	 * @param requested_class the class requested
	 * @param announcedata extra data to create the class, this must be fully read
	 * @param role the role for this class
	 * @param net_id the network id of this new node
	 */
	virtual void onNodeDynamic(ZCom_ClassID requested_class, ZCom_BitStream* announcedata,
			eZCom_NodeRole role, ZCom_NodeID net_id);

};

}

#endif
