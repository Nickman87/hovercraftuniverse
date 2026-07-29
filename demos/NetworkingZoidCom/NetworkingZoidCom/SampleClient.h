#pragma once

#include "NetworkClient.h"
#include "NetworkEntity.h"
#include "SampleEventParser.h"
#include <vector>

using HovUni::NetworkEntity;
using HovUni::SampleEventParser;
using std::vector;

class Player;

class SampleClient: public HovUni::NetworkClient {
public:
	/**
	 * Constructor for remote connection
	 *
	 * @param name the name of the server
	 */
	SampleClient(const char* name);

	/**
	 * Constructor for local connection
	 */
	SampleClient();

	/**
	 * Destructor
	 */
	~SampleClient();

	/**
	 * Process incoming and outgoing packets
	 */
	virtual void process();

	/**
	 * Register classes with this server
	 */
	virtual void registerClasses();

	/**
	 * Whether the client can exit because the connection was lost
	 *
	 * @return True if the client can exit, false otherwise.
	 */
	bool canExit() const;

	/**
	 * Send a move forward event
	 */
	void moveForward();

	/**
	 * Send a move backward event
	 */
	void moveBackward();

	/**
	 * Send a move left event
	 */
	void moveLeft();

	/**
	 * Send a move right event
	 */
	void moveRight();

	/**
	 * Add an entity to the list
	 *
	 * @param entity the entity
	 */
	void addEntity(NetworkEntity* entity);

private:
	bool mExit;
	Player* mPlayer;
	vector<NetworkEntity*> mEntities;

//
// ZCom_Control callbacks
//
public:
	/**
	 * Connection process finished (Client).
	 *
	 * @param id The connection's ID
	 * @param result The result of the connection process
	 * @param reply The reply given by the destination
	 */
	void ZCom_cbConnectResult(ZCom_ConnID id, eZCom_ConnectResult result, ZCom_BitStream& reply);

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
	 * ZoidLevel migration process finished (Server, Client).
	 *
	 * @param id The connection's ID
	 * @param result Result of the migration process
	 * @param new_level New ZoidLevel of the connection
	 * @param reason Additional data passed by the server
	 */
	void ZCom_cbZoidResult(ZCom_ConnID id, eZCom_ZoidResult result, zU8 new_level, ZCom_BitStream& reason);

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

};
