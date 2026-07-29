#ifndef HUCLIENT_H_
#define HUCLIENT_H_

#include "NetworkClient.h"
#include "EntityManager.h"
#include "NetworkIDManager.h"
#include "Lobby.h"
#include "LobbyListener.h"
#include <ChatClient.h>
#include <ChatListener.h>

#include <boost/interprocess/sync/interprocess_semaphore.hpp>

namespace HovUni {

/**
 * This class represents the core of the client. It holds the controls
 * for the networking etc.
 *
 * @author Olivier Berghmans & Nick De Frangh
 */
class HUClient: public NetworkClient {
private:
	/** The address to which this client is/was connecting */
	Ogre::String mAddress;

	/** The unique ID of the client */
	ZCom_ConnID mID;

	/** The entity manager */
	EntityManager * mEntityManager;

	/** The entity manager */
	NetworkIDManager* mIDManager;

	/** The lobby */
	Lobby* mLobby;

	/** The scene manager */
	Ogre::SceneManager * mSceneMgr;

	/** A chatlistener */
	ChatListener* mChatListener;

	/** A chat client associated with this server */
	ChatClient* mChatClient;

	/** A Semaphore to make it possible to wait for complete connection */
	boost::interprocess::interprocess_semaphore mSemaphore;

	/** Boolean to check if the client has finished connection, either good or bad */
	bool mFinishedConnecting;

public:
	/**
	 * Constructor for a client using a remote connection
	 *
	 * @param name the name of the server
	 * @param port the port of the server
	 * @param playername
	 */
	HUClient(const char* name, unsigned int port = 2375);

	/**
	 * Constructor for a client using a local connection. This client
	 * must be running in the same instance of a program as the server. 
	 * It is used for combining the client and server to avoid the
	 * overhead of real network.
	 */
	HUClient();

	/**
	 * Destructor
	 */
	~HUClient();

	/**
	 * Process incoming and outgoing packets
	 *
	 * @param simulationTimePassed the time that elapsed since the last process (milliseconds)
	 */
	virtual void process(zU32 simulationTimePassed);

	/**
	 * Disconnect
	 *
	 * @param reason the reason of the disconnect
	 */
	void disconnect(const std::string& reason);

	/**
	 * Get the unique ID for the client
	 *
	 * @return the unique ID
	 */
	unsigned int getID() const;

	/**
	 * Request the current chat client
	 */
	ChatClient* getChat() {
		return mChatClient;
	}

	/**
	 * Request the lobby object
	 */
	Lobby* getLobby() {
		return mLobby;
	}

	/**
	 * Set a chatlistener on the client. This function will delay the registration
	 * of the listener until the chat client is registered.
	 *
	 * @param listener The listener to register
	 */
	void setChatListener(ChatListener* listener);

	/**
	 * Remove a chatlistener from the chat client.
	 *
	 * @param listener The listener to remove
	 */
	void removeChatListener(ChatListener* listener);

	/**
	 * Wait for the client to fully connect to the server
	 */
	void wait();

	/**
	 * Wati for the client to fully connect to the server, with
	 * a specific timeout
	 *
	 * @param abs_time The time on which you want to unlock
	 */
	void timed_wait(const boost::posix_time::ptime & abs_time);

	/**
	 * Check if the client has finished connecting
	 *
	 * @return true if it has finished
	 */
	inline bool finishedConnecting() const {
		return mFinishedConnecting;
	}

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
	virtual void onNodeDynamic(ZCom_ClassID requested_class, ZCom_BitStream* announcedata, eZCom_NodeRole role,
			ZCom_NodeID net_id);

};

}

#endif
