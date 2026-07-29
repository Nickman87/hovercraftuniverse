namespace HovUni {

template<typename Key, typename Player, bool Unique>
PlayerMap<Key, Player, Unique>::PlayerMap() :
	mOwnPlayer(0) {
}

template<typename Key, typename Player, bool Unique>
PlayerMap<Key, Player, Unique>::~PlayerMap() {

}

template<typename Key, typename Player, bool Unique>
void PlayerMap<Key, Player, Unique>::addPlayer(const Key& key, Player* player, bool ownPlayer) {
	if (Unique) {
		removePlayerByKey(key);
	}

	// Insert the player at the back
	mPlayers.push_back(std::pair<Key, Player*>(key, player));

	// Set own player
	if (ownPlayer) {
		mOwnPlayer = player;
	}
}

template<typename Key, typename Player, bool Unique>
void PlayerMap<Key, Player, Unique>::removePlayerByKey(const Key& key) {
	list_type::iterator it = find(key);
	removePlayerByIterator(it);
}

template<typename Key, typename Player, bool Unique>
typename PlayerMap<Key, Player, Unique>::list_type::iterator PlayerMap<Key, Player, Unique>::removePlayerByIterator(
		typename list_type::iterator it) {
	// Delete the player
	delete it->second;
	// Remove from the list
	return mPlayers.erase(it);
}

template<typename Key, typename Player, bool Unique>
Player* PlayerMap<Key, Player, Unique>::getPlayer(const Key& key) const {
	list_type::const_iterator it = find(key);
	Player* ret = 0;
	if (it != mPlayers.end()) {
		ret = it->second;
	}
	return ret;
}

template<typename Key, typename Player, bool Unique>
typename PlayerMap<Key, Player, Unique>::list_type::iterator PlayerMap<Key, Player, Unique>::find(const Key& key) {
	for (list_type::iterator it = mPlayers.begin(); it != mPlayers.end(); ++it) {
		if (it->first == key) {
			return it;
		}
	}
	return mPlayers.end();
}

template<typename Key, typename Player, bool Unique>
typename PlayerMap<Key, Player, Unique>::list_type::const_iterator PlayerMap<Key, Player, Unique>::find(const Key& key) const {
	for (list_type::const_iterator it = mPlayers.begin(); it != mPlayers.end(); ++it) {
		if (it->first == key) {
			return it;
		}
	}
	return mPlayers.end();
}

template<typename Key, typename Player, bool Unique>
typename PlayerMap<Key, Player, Unique>::list_type::iterator PlayerMap<Key, Player, Unique>::begin() {
	return mPlayers.begin();
}

template<typename Key, typename Player, bool Unique>
typename PlayerMap<Key, Player, Unique>::list_type::const_iterator PlayerMap<Key, Player, Unique>::begin() const {
	return mPlayers.begin();
}

template<typename Key, typename Player, bool Unique>
typename PlayerMap<Key, Player, Unique>::list_type::iterator PlayerMap<Key, Player, Unique>::end() {
	return mPlayers.end();
}

template<typename Key, typename Player, bool Unique>
typename PlayerMap<Key, Player, Unique>::list_type::const_iterator PlayerMap<Key, Player, Unique>::end() const {
	return mPlayers.end();
}

}
