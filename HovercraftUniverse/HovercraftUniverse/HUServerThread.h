#ifndef HUSERVERTHREAD_H_
#define HUSERVERTHREAD_H_

#include "HUServerCore.h"

namespace HovUni {

/**
 * This class represents the server thread for the server.
 *
 * @author Olivier Berghmans
 */
class HUServerThread {
private:
	/** The server core */
	HUServerCore* mServerCore;

	/** Whether to stop the thread */
	bool mStop;

public:
	/**
	 * Constructor
	 */
	HUServerThread();

	/**
	 * Destructor
	 */
	~HUServerThread();

	/**
	 * Indicate the thread to stop
	 */
	void stop();

	/**
	 * Call operator used for threading
	 */
	void operator()();
};

}

#endif
