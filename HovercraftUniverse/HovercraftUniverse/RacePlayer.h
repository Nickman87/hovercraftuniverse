#ifndef RACEPLAYER_H
#define RACEPLAYER_H

#include "NetworkEntity.h"
#include <map>

namespace HovUni {

class Lobby;
class RaceState;
class PlayerSettings;

/**
 * Class that represents a player in the race. It will hold his current
 * position, etc.
 *
 * @author Olivier Berghmans
 */
class RacePlayer: public NetworkEntity, public ZCom_NodeReplicationInterceptor {
private:
	/** The race state */
	RaceState* const mRaceState;

	/** The general player settings and data */
	PlayerSettings* const mPlayerSettings;

	/** The current position of this player in the race */
	unsigned int mPlayerPosition;

	/** Whether the player finished */
	bool mFinished;

	/** The last checkpoint reached */
	unsigned int mLastCheckpoint;

	/** The timestamps of the checkpoints reached */
	std::map<unsigned int, long> mCheckpoints;

public:
	/**
	 * Constructor
	 *
	 * @param state the race state
	 * @param playerSettings The general settings for this player
	 * @param position the position of the player
	 */
	RacePlayer(RaceState* state, PlayerSettings* playerSettings, int position);

	/**
	 * Constructor
	 *
	 * @param lobby the lobby to retrieve the race state and player settings from
	 * @param announcementdata the announce data
	 * @param ID the class ID
	 * @param control the network control
	 */
	RacePlayer(Lobby* lobby, ZCom_BitStream* announcementdata, ZCom_ClassID id,
			ZCom_Control* control);

	/**
	 * Destructor
	 */
	~RacePlayer();

	/**
	 * Get the race state for this player
	 *
	 * @return the race state
	 */
	RaceState* getRaceState();

	/**
	 * Get the player settings for this player
	 *
	 * @return the settings
	 */
	PlayerSettings* getSettings();

	/**
	 * Check whether this is a bot
	 *
	 * @return true if it is a bot, false otherwise
	 */
	bool isBot() const;

	/**
	 * Set the current position of the player
	 *
	 * @param position the new position
	 */
	void setPosition(short position);

	/**
	 * Get the current position of the player
	 *
	 * @return the current position
	 */
	short getPosition() const;

	/**
	 * Indicate that the player finished
	 */
	void setFinished();

	/**
	 * Check whether the player finished
	 *
	 * @return true if finished, false otherwise
	 */
	bool isFinished() const;

	/**
	 * Add a timestamp of a checkpoint reached by this player
	 *
	 * @param checkpoint the ID of the checkpoint
	 * @param timestamp the timestamp
	 */
	void addCheckpoint(unsigned int checkpoint, long timestamp);

	/**
	 * Get the timestamp for a checkpoint
	 *
	 * @param checkpoint the ID of the checkpoint
	 * @return the timestamp, or -1 if the checkpoint was not reached yet
	 */
	long getCheckpoint(unsigned int checkpoint);

	/**
	 * Get the ID of the last checkpoint reached
	 *
	 * @return the ID of the last checkpoint reached
	 */
	unsigned int getLastCheckpoint();

	/**
	 * Get the class name for this class. This is used for registering
	 * the class with the network
	 *
	 * @return the class name
	 */
	static std::string getClassName();

protected:

	/**
	 * A callback that should be implemented in order to parse and process
	 * incoming events.
	 */
	virtual void parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id,
			ZCom_BitStream* stream, float timeSince);

	/**
	 * A callback that should be implemented so the replicators for this
	 * entity can be set up. In case the number of replicators given to the
	 * constructor is zero, this callback can be implemented as empty.
	 */
	virtual void setupReplication();

	/**
	 * A callback that should be implemented so the announcement data for this
	 * entity can be set.
	 *
	 * @param stream the bitstream where the data can be set
	 */
	virtual void setAnnouncementData(ZCom_BitStream* stream);

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

#endif //_RACEPLAYER_H
