#ifndef RACESTATE_H
#define RACESTATE_H

#include "NetworkEntity.h"
#include "Listenable.h"
#include "PlayerMap.h"
#include "Config.h"
#include <iostream>
#include <set>
#include <boost/random.hpp>

namespace HovUni {

class Lobby;
class Loader;
class ClientLoader;
class ClientPreparationLoader;
class RacePlayer;
class RaceStateListener;
class Timing;
class Finish;
class CheckPoint;
class Start;
class Hovercraft;

/**
 * RaceState will be the main controlling component during the game.
 * It will manage the game state, handle game events and take care of all the
 * game logic.
 *
 * @author Olivier Berghmans & Nick De Frangh
 */
class RaceState: public NetworkEntity, public Listenable<RaceStateListener> {
public:
	/** The player map type */
	typedef PlayerMap<unsigned int, RacePlayer, false> playermap;

	/** All the possible states during the race */
	static const enum States {
		INITIALIZING = 0, /** Pre-race state, nothing has been loaded yet */
		LOADING, /** Server and clients load the world */
		INTRO, /** Intro for the race */
		COUNTDOWN, /** The race will start, show countdown */
		RACING, /** The race has started */
		FINISHING, /** Someone has finished, wait for everyone to finish */
		OUTRO, /** Outro for the race to show rankings */
		CLEANUP
	/** Clean up and return to lobby */
	};

	/** All the possible events the client can send to the server during the race */
	static const enum Events {
		INITIALIZED = 0, LOADED, STATECHANGED
	};

	/*
	 * Not replicated fields
	 */
private:
	/** The finite state machine for this race state */
	class SystemState;

	/** The player map */
	playermap mPlayers;

	/** The current state of the race (Server + Client) */
	SystemState* mState; // Replicated through events not replicators

	/** Whether this is the server object or not (Server + Client) */
	bool mServer;

	/** The reference to the lobby (Server + Client) */
	Lobby* const mLobby;

	/** The loader (Server + Client) */
	Loader* const mLoader;

	/** The countdown interval */
	const long mCountdownInterval;
	const unsigned char mCountdownIntervalLog2;

	/** Map with player ID's and next checkpoint ID (Server) */
	std::map<unsigned int, Ogre::int32> mCheckpointMapping;

	/** ID of the finish (= last checkpoint + 1) (Server) */
	Ogre::int32 mFinishID;

	/** The positions of the players */
	std::list<unsigned int> mPlayerPositions;

	/** Random number generator */
	boost::mt19937* mRng;

	/** The introtime */
	int mIntroTime;

	/** The playtime */
	int mPlayTime;

	/** The finish time */
	int mFinishTime;



	/*
	 * Replicated fields
	 */
private:
	// Adding a replicated field: set up replication (in setupReplication) + init event (in parseEvents)
	// Const fields should be in announce data and not as replicator

	/** The track file name for this race */
	const Ogre::String mTrackFilename;

	/** The number of players */
	int mNumberPlayers;

	/** The countdown variable */
	int mCountdown;

public:
	/**
	 * Constructor
	 *
	 * @param lobby a reference to the lobby
	 * @param loader the loader for the server
	 * @param track the track filename
	 */
	RaceState(Lobby* lobby, Loader* loader, Ogre::String track);

	/**
	 * Constructor
	 *
	 * @param lobby a reference to the lobby
	 * @param announcementdata the announce data
	 * @param ID the class ID
	 * @param control the network control
	 */
	RaceState(Lobby* lobby, ClientPreparationLoader* loader, ZCom_BitStream* announcementdata, ZCom_ClassID id,
			ZCom_Control* control);

	/**
	 * Destructor
	 */
	virtual ~RaceState();

	/**
	 * Process the race state
	 */
	void process();

	/**
	 * Get the track filename (Server + Client)
	 *
	 * @return the filename of the track
	 */
	inline Ogre::String getTrackFilename() const {
		return mTrackFilename;
	}

	/**
	 * Get a player from the lobby
	 *
	 * @param id the id of the player
	 * @return the settings
	 */
	inline RacePlayer* getPlayer(unsigned int id) const {
		return mPlayers.getPlayer(id);
	}

	/**
	 * Get a map with all players mapped on their connection id (Server + Client)
	 *
	 * @return player settings mapped on their connection id
	 */
	inline const playermap::list_type& getPlayers() const {
		return mPlayers.getPlayers();
	}

	/**
	 * Get the own player
	 *
	 * @return the own player settings
	 */
	inline RacePlayer* getOwnPlayer() {
		return mPlayers.getOwnPlayer();
	}

	/**
	 * Get the state of the race state
	 *
	 * @return the state
	 */
	States getState() const;

	/**
	 * Check if the race is done, i.e. the race state is clean up
	 *
	 * @return true if the race is done, false otherwise
	 */
	bool isDone() const;

	/**
	 * Get the countdown timer which indicates the time left on the timer
	 *
	 * @return the remaining time in milliseconds, or -1 if there is no countdown (not in countdown state)
	 */
	inline int getCountdown() {
		return mCountdown;
	}

	/**
	 * Add a player to the race state
	 *
	 * @param player the player settings
	 * @param ownPlayer indicates whether it is the setting of the own player
	 */
	void addPlayer(RacePlayer* player, bool ownPlayer = false);

	/**
	 * Remove player connection id
	 *
	 * @param id the ID of the player
	 */
	void removePlayer(unsigned int id);

	// Game events that are triggered on server propagated from havok //

	/**
	 * Called when player leaves the finish box
	 *
	 * @param finish
	 * @param playerid
	 */
	void onFinish(Finish * finish, unsigned int playerid);

	/**
	 * Called when player leaves the checkpoint box
	 *
	 * @param checkpoint
	 * @param playerid
	 */
	void onCheckPoint(CheckPoint * checkpoint, unsigned int playerid);

	/**
	 * Called when start leaves the checkpoint box
	 *
	 * @param start
	 * @param playerid
	 */
	void onStart(Start * start, unsigned int playerid);

	/**
	 * Get the class name for this class. This is used for registering
	 * the class with the network
	 *
	 * @return the class name
	 */
	static std::string getClassName();

	/**
	 * Get the list of listeners
	 *
	 * @return the list
	 */
	Listenable<RaceStateListener>::list_type& getListeners();

	/**
	 * Indicate that the loading is done. (Client)
	 */
	void onLoaded();

protected:

	/**
	 * Remove player
	 *
	 * @param i the iterator pointing to the player
	 */
	playermap::iterator removePlayer(playermap::iterator i);

	/**
	 * Indicate that the initialization is done. (Client)
	 */
	void onInitialized();

	/**
	 * Update the checkpoint of a player
	 *
	 * @param playerid the ID of the player
	 */
	void updatePlayerCheckpoint(unsigned int playerid);

	/**
	 * A callback that should be implemented in order to parse and process
	 * incoming events.
	 */
	virtual void parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id, ZCom_BitStream* stream,
			float timeSince);

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

private:
	/**
	 * Calculate and update new positions because of a player reaching a checkpoint
	 *
	 * @param playerid the ID of the player that reached a checkpoint
	 */
	void calculatePlayerPosition(unsigned int playerid);

	/**
	 * Get the random settings for a bot given some keymaps
	 *
	 * @param hovercrafts the key map of hovercraft keys
	 * @param characters the key map of character keys
	 * @param hovercraftKey the random key for the hovercraft (OUT)
	 * @param characterKey the random key for the character (OUT)
	 */
	void randomSettingsForBot(const std::map<unsigned int,Ogre::String>& hovercrafts, const std::map<unsigned int,Ogre::String>& characters, unsigned int& hovercraftKey,
			unsigned int& characterKey);

	/**
	 * This sub-class represents the finite state machine of the race state.
	 *
	 * @author Olivier Berghmans
	 */
	class SystemState {
	private:
		/** Reference to the race state */
		RaceState* const mRaceState;

		/** The current state of the race state */
		States mCurrentState;

		/** List of player IDs where the server is currently waiting on (Server) */
		std::set<unsigned int> mWaitingList;

		/** Whether the server is waiting for the clients to renew their state (Server) */
		bool mStartOfState;

	public:
		/** The timer used for the countdown (Server) */
		Timing* mTimer;

		/** The timer used for the playtime (Server) */
		Timing* mPlayTimer;

		/**
		 * Constructor
		 *
		 * @param racestate the race state
		 */
		SystemState(RaceState* racestate);

		/**
		 * Destructor
		 */
		~SystemState();

		/**
		 * Update the system state
		 */
		void update();

		/**
		 * Get the current state
		 */
		States getState() const;

		/**
		 * Set the new state for the race state. (Server + Client)
		 *
		 * @param state the new state
		 */
		void newState(States state);

		/**
		 * A new event has arrived. (Server)
		 *
		 * @param event the event
		 * @param id the ID of the player who sent the vent
		 */
		void newEvent(Events events, unsigned int id);

		/**
		 * Send an event to the server. (Client)
		 *
		 * @param event the event
		 */
		void sendEvent(Events events);

		/**
		 * Callback for when the race state has gone into loading state. (Server + Client)
		 */
		void onLoading();

		/**
		 * Fill the waiting list with the current players. (Server)
		 */
		void setWaitingList();

		/**
		 * Remove an ID from the waiting list and if the list becomes empty, do the correct
		 * action.
		 *
		 * @param id the ID to remove
		 */
		void eraseFromList(unsigned int id);

	};
};

/**
 * Output operator for the race state
 *
 * @param os the output stream
 * @param state the race state
 * @return the updated output stream
 */
std::ostream& operator<<(std::ostream& os, const RaceState::States& state);

/**
 * Output operator for the race state event
 *
 * @param os the output stream
 * @param state the race state event
 * @return the updated output stream
 */
std::ostream& operator<<(std::ostream& os, const RaceState::Events& events);

}

#endif //_RACESTATE_H
