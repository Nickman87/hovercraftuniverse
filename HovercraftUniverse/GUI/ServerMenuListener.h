#ifndef _SERVERMENULISTENER_H
#define _SERVERMENULISTENER_H

#include <OgreString.h>
#include "ConnectListener.h"

namespace HovUni {
	/**
	 * An interface for a serverMenuListener. This should be implemented by
	 * classes who are interested in the server menu events.
	 *
	 * @author Nick De Frangh
	 */
	class ServerMenuListener {
		public:
			/**
			 * Function that will be called when a client wants to connect to a certain address.
			 *
			 * @param address The IP:Port string represention the server address.
			 * @param listener The listener to notify the end of the connection to
			 */
			virtual void onConnect(const Ogre::String& address, ConnectListener* listener) = 0;

			/**
			 * When a player wants to create a new game, this method will be called.
			 */
			virtual void onCreate() = 0;

			/**
			 * Call this function when the connection has been succesfull and you want
			 * to switch state
			 */
			virtual void finishConnect() = 0;
	};
}

#endif //_SERVERMENULISTENER_H