#ifndef PLAYERMAP_H
#define PLAYERMAP_H

#include <vector>
#include <set>

namespace HovUni {

/**
 * The player map. It can hold a map with players. The type of the key and the type
 * of the player can be given, and also whether the key has to be unique or not
 *
 * @author Olivier Berghmans
 */
template<typename Key, typename Player, bool Unique>
class PlayerMap {
public:
	/** The type of the keys */
	typedef Key key_type;

	/** The type of the players */
	typedef Player player_type;

	/** The list type */
	typedef typename std::vector<std::pair<Key, Player*> > list_type;

	/** The iterator type */
	typedef typename list_type::iterator iterator;
	typedef typename list_type::const_iterator const_iterator;

protected:
	/** The map with the players */
	list_type mPlayers;

	/** The reference to the own player, can be used for quickly access */
	Player* mOwnPlayer;

public:
	/**
	 * Constructor
	 */
	PlayerMap();

	/**
	 * Destructor
	 */
	~PlayerMap();

	/**
	 * Get a list with players. The list holds pairs with the key
	 * and the player.
	 *
	 * @return list of players stored with their key in a pair
	 */
	inline const list_type& getPlayers() const {
		return mPlayers;
	}

	/**
	 * Get the own player
	 *
	 * @return the own player
	 */
	inline Player* getOwnPlayer() const {
		return mOwnPlayer;
	}

	/**
	 * Get a player
	 *
	 * @param key the key of the player
	 * @return the player
	 */
	Player* getPlayer(const Key& key) const;

	/**
	 * Add a player and delete the previous player with the same key if Unique is true
	 *
	 * @param key the key for the player
	 * @param player the player
	 * @param ownPlayer indicates whether it is the setting of the own player
	 */
	void addPlayer(const Key& key, Player* player, bool ownPlayer = false);

	/**
	 * Remove a player and delete its object
	 *
	 * @param key the key of the player
	 */
	void removePlayerByKey(const Key& key);

	/**
	 * Remove a player and delete its object
	 *
	 * @param it the iterator pointing to the player
	 * @return the iterator to the next player
	 */
	typename list_type::iterator removePlayerByIterator(typename list_type::iterator it);

	/**
	 * Find a player with a certain key
	 *
	 * @return the iterator pointing to the player
	 */
	typename list_type::iterator find(const Key& key);

	/**
	 * Find a player with a certain key
	 *
	 * @return the iterator pointing to the player
	 */
	typename list_type::const_iterator find(const Key& key) const;

	/**
	 * Get the begin iterator
	 *
	 * @return the begin iterator
	 */
	typename list_type::iterator begin();

	/**
	 * Get the begin iterator
	 *
	 * @return the begin iterator
	 */
	typename list_type::const_iterator begin() const;

	/**
	 * Get the end iterator
	 *
	 * @return the end iterator
	 */
	typename list_type::iterator end();

	/**
	 * Get the end iterator
	 *
	 * @return the end iterator
	 */
	typename list_type::const_iterator end() const;

};

}

#include "PlayerMapImpl.h"

#endif // PLAYERMAP_H
