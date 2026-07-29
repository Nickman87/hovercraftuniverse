namespace HovUni {

template<typename ListenerType>
void Listenable<ListenerType>::addListener(ListenerType* listener) {
	mListeners.push_back(listener);
}

template<typename ListenerType>
void Listenable<ListenerType>::removeListener(ListenerType* listener) {
	listener_iterator it = listenersBegin();

	while (it != listenersEnd()) {
		if ((*it) == listener) {
			mListeners.erase(it);
			return;
		}

		++it;
	}
}

template<typename ListenerType>
void Listenable<ListenerType>::listenersClear() {
	mListeners.clear();
}

template<typename ListenerType>
typename Listenable<ListenerType>::listener_iterator Listenable<ListenerType>::listenersBegin() {
	return mListeners.begin();
}

template<typename ListenerType>
typename Listenable<ListenerType>::listener_const_iterator Listenable<ListenerType>::listenersBegin() const {
	return mListeners.begin();
}

template<typename ListenerType>
typename Listenable<ListenerType>::listener_iterator Listenable<ListenerType>::listenersEnd() {
	return mListeners.end();
}

template<typename ListenerType>
typename Listenable<ListenerType>::listener_const_iterator Listenable<ListenerType>::listenersEnd() const {
	return mListeners.end();
}

}
