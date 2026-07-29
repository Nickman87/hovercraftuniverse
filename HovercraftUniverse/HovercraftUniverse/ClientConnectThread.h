#ifndef _CLIENTCONNECTTHREAD_H
#define _CLIENTCONNECTTHREAD_H

#include <boost/thread/thread.hpp>
#include "HUClient.h"
#include "ConnectListener.h"

namespace HovUni {
	class ClientConnectThread {
		private:
			/** A reference to the client that wants connecting */
			HUClient* mClient;

			/** The listener for this connection attempt */
			ConnectListener* mListener;

			/** The boost thread container */
			boost::thread* mThread;

		public:
			/**
			 * Constructor
			 *
			 * @param client The client that has to get connected
			 * @param listener The object to inform about a successfull or failed connection.
			 */
			ClientConnectThread(HUClient* client, ConnectListener* listener);

			/**
			 * Destructor
			 */
			~ClientConnectThread();

			/**
			 * Start connecting
			 */
			void start();

			/**
			 * Call operator used for threading
			 */
			void operator()();
	};
}

#endif //_CLIENTCONNECTTHREAD_H