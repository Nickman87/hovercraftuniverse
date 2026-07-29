#ifndef LOBBY_H
#define LOBBY_H

#include "NetworkEntity.h"
#include "GameEvent.h"
#include "Listenable.h"
#include "PlayerMap.h"
#include <map>
#include <string>

namespace HovUni {

class LobbyListener;
class Loader;
class PlayerSettings;
class RaceState;
class Timing;
class ChatServer;

/**
 * The Lobby which holds the settings of the connected players, assigns an
 * administrator and holds the map info. It also contains a custom loader that is used to load the initial world.
 *
 * @author Olivier Berghmans & Pieter-Jan Pintens
 */
class Lobby: public NetworkEntity,
		public Listenable<LobbyListener> ,
		public ZCom_NodeReplicationInterceptor {
public:
	/** The player map type */
	typedef PlayerMap<unsigned int, PlayerSettings, false> playermap;

	/*
	 * Not replicated fields
	 */
private:
	/** The player map */
	playermap mPlayers;

	/** Specific loader, will be different on client and server **/
	Loader * mLoader;

	/** Indicator of whether there is an administrator */
	bool mHasAdmin;

	/** Whether the start was initiated (Server) */
	bool mPressedStart;

	/** The timer used for the countdown (Server) */
	Timing* mTimer;

	/** Countdown value for the start (Server) */
	int mCountdown;

	/** Total countdown */
	static int msCountdownValue;

	/** The chat server (Server) */
	ChatServer* mChatServer;

	/** The race state when a race is busy */
	RaceState* mRaceState;

	/*
	 * Replicated fields
	 */
private:
	// Adding a replicated field: set up replication (in setupReplication) + init event (in parseEvents)

	/** The ID of the administrator */
	unsigned int mAdmin;

	/** The maximum number of players allowed */
	unsigned int mMaximumPlayers;

	/** The current number of players */
	unsigned int mCurrentPlayers;

	/** The id of the track */
	unsigned int mTrack;

	/** Whether the lobby has bots or not */
	bool mBots;

public:

	/**
	 * Constructor
	 *
	 * @param loader the loader for the loby, will be deleted by the lobby on destruction
	 */
	Lobby(Loader* loader);

	/**
	 * Destructor
	 */
	~Lobby();

	/**
	 * Process the lobby. This will process all events for the lobby and for all the
	 * players in the lobby. (Server + Client)
	 */
	void process();

	/**
	 * Start the track. (Client: Owner)
	 */
	void start();

	/**
	 * Check if the client is the administrator of the lobby. (Client)
	 */
	bool isAdmin() const;

	/**
	 * Add a player setting to the lobby. (Server + Client)
	 *
	 * @param settings the player settings
	 * @param ownPlayer indicates whether it is the setting of the own player
	 */
	void addPlayer(PlayerSettings * settings, bool ownPlayer = false);

	/**
	 * Get a player setting from the lobby. (Server + Client)
	 *
	 * @param id the id of the player
	 * @return the settings
	 */
	inline PlayerSettings* getPlayer(unsigned int id) {
		return mPlayers.getPlayer(id);
	}

	/**
	 * Get the player ID for a given connection ID
	 *
	 * @param connID the connection ID of the player
	 * @return the unique ID for the player
	 */
	unsigned int getPlayerIDfromConnectionID(ZCom_ConnID connID) const;

	/**
	 * Get the class name for this class. This is used for registering
	 * the class with the network
	 *
	 * @return the class name
	 */
	static std::string getClassName();

	/**
	 * Get a map with all players mapped on their connection id (Server + Client)
	 *
	 * @return player settings mapped on their connection id
	 */
	inline const playermap::list_type& getPlayers() const {
		return mPlayers.getPlayers();
	}

	/**
	 * Get the own player settings (Client)
	 *
	 * @return the own player settings
	 */
	inline PlayerSettings* getOwnPlayer() const {
		return mPlayers.getOwnPlayer();
	}

	/**
	 * Get the track id (Server + Client)
	 *
	 * @return the id of the track
	 */
	inline unsigned int getTrackId() const {
		return mTrack;
	}

	/**
	 * Set the track id (Client: Owner)
	 *
	 * @param trackid the new id of the track
	 */
	inline void setTrackId( unsigned int trackid ) {
		mTrack = trackid;
	}

	/**
	 * Get the Id of the current admin
	 *
	 * @return The id of the current admin
	 */
	inline unsigned int getAdminId() const {
		return mAdmin;
	}

	/**
	 * Set the race state. The state will be deleted by the lobby itself. (Server + Client)
	 *
	 * @param state the new race state
	 */
	void setRaceState(RaceState* state);

	/**
	 * Get the race state. (Server + Client)
	 *
	 * @return the race state
	 */
	RaceState* getRaceState();

	/**
	 * Called when player is about to connect. (Server)
	 *
	 * @param id the ID of the new connected player
	 * @return true when there is room for a player, false if Lobby is full
	 */
	bool onConnectAttempt(ZCom_ConnID id);

	/**
	 * Called when player connects. (Server)
	 *
	 * @param id the ID of the new connected player
	 */
	virtual void onConnect(ZCom_ConnID id);

	/**
	 * Called when player disconnects. (Server)
	 * 
	 * @param id the ID of the disconnected player
	 * @param reason the reason
	 */
	virtual void onDisconnect(ZCom_ConnID id, const std::string& reason);

	/**
	 * Called when admin sends start. (Server)
	 */
	virtual void onStartServer();

	/**
	 * Called to notify the listeners of the start. (Client)
	 */
	virtual void onStartClient();

	/**
	 * Mark the lobby to fill the server with bots or not. (Client: Owner)
	 *
	 * @param val True when you want to fill with bots, false otherwise
	 */
	inline void setBots(bool val) {
		mBots = val;
	}

	/**
	 * Check if we want to fill with bots or not (Server + Client)
	 *
	 * @return True when we want to fill, false otherwise
	 */
	inline bool hasBots() const {
		return mBots;
	}

	/**
	 * Set the maximum number of players allowed in the lobby. (Client: Owner)
	 *
	 * @param max The maximum number of players allowed
	 */
	inline void setMaxPlayers(unsigned int max) {
		mMaximumPlayers = max;
	}

	/**
	 * Request the maximum number of players allowed. (Server + Client)
	 *
	 * @return The maximum number of players allowed
	 */
	inline unsigned int getMaxPlayers() const {
		return mMaximumPlayers;
	}

	/**
	 * Request the number of connected players. (Server + Client)
	 *
	 * @return The number of real players
	 */
	inline unsigned int getNumberOfPlayers() const {
		return mCurrentPlayers;
	}

	/**
	 * Set the chat server so that the lobby can send notifications
	 *
	 * @param chat the chat server
	 */
	inline void setChatServer(ChatServer* chat) {
		mChatServer = chat;
	}

protected:

	/**
	 * Remove player id. (Server + Client)
	 *
	 * @param id the ID of the player
	 */
	void removePlayer(unsigned int id);

	/**
	 * Remove player. (Server + Client)
	 *
	 * @param i the iterator pointing to the player
	 */
	playermap::iterator removePlayer(playermap::iterator i);

	//OVERWRITEN FROM NetworkEntity

	/*
	 * @see NetworkEntity::parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id, ZCom_BitStream* stream, float timeSince)
	 */
	virtual void parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id, ZCom_BitStream* stream,
			float timeSince);

	/*
	 * @see NetworkEntity::setupReplication()
	 */
	virtual void setupReplication();

	/*
	 * @see NetworkEntity::setAnnouncementData(ZCom_BitStream* stream)
	 */
	virtual void setAnnouncementData(ZCom_BitStream* stream);

	/**
	 * Process a game event at the server. (Server)
	 *
	 * @param event an event
	 */
	void processEventsServer(GameEvent* gEvent);

	/**
	 * Process a game event at the owner. (Client: Owner)
	 *
	 * @param event an event
	 */
	void processEventsOwner(GameEvent* gEvent);

	/**
	 * Process a game event at other clients. (Client: Others)
	 *
	 * @param event an event
	 */
	void processEventsOther(GameEvent* gEvent);

private:
	/**
	 * Delete the race state
	 */
	void deleteRaceState();

	///////////////////////////
	// INTERCEPTOR FUNCTIONS //
	///////////////////////////
public:
	/**
	 * @inheritDoc
	 */
	bool inPreUpdate(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role) {
		return true;
	}

	/**
	 * @inheritDoc
	 */
	bool inPreUpdateItem(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role, ZCom_Replicator *_replicator,
			zU32 _estimated_time_sent) {
		return true;
	}

	/**
	 * @inheritDoc
	 */
	void inPostUpdate(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role, zU32 _rep_bits, zU32 _event_bits,
			zU32 _meta_bits);

	/**
	 * @inheritDoc
	 */
	void outPreReplicateNode(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) {
	}

	/**
	 * @inheritDoc
	 */
	void outPreDereplicateNode(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) {
	}

	/**
	 * @inheritDoc
	 */
	bool outPreUpdate(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) {
		return true;
	}

	/*! @copydoc ZCom_NodeReplicationInterceptor::outPreUpdateItem(ZCom_Node*,ZCom_ConnID,eZCom_NodeRole,ZCom_Replicator*) */
	bool outPreUpdateItem(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role, ZCom_Replicator *_replicator) {
		return true;
	}

	/*! @copydoc ZCom_NodeReplicationInterceptor::outPostUpdate(ZCom_Node*,ZCom_ConnID,eZCom_NodeRole,zU32,zU32,zU32) */
	void outPostUpdate(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role, zU32 _rep_bits, zU32 _event_bits,
			zU32 _meta_bits) {
	}

};

}

#endif
