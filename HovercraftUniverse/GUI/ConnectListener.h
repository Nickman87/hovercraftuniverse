#ifndef _CONNECTLISTENER_H
#define _CONNECTLISTENER_H

namespace HovUni {
	/**
	 * Interface for a connection listener. Should be implemented
	 * by classes that want to listen for a connection to finish.
	 *
	 * @author Nick De Frangh
	 */
	class ConnectListener {
		public:
			/**
			 * Function that will be called when the connection attempt has stopped.
			 *
			 * @param success Will notify of a succesfull connection or a failed one.
			 */
			virtual void onConnectFinish(bool success) = 0;
	};
}

#endif //_CONNECTLISTENER_H