#ifndef HUSERVER_H_
#define HUSERVER_H_

#include "HUServerThread.h"
#include <boost/thread/thread.hpp>

namespace HovUni {

/**
 * This class represents the server.
 *
 * @author Olivier Berghmans
 */
class HUServer {
private:
	/** The boost thread container */
	boost::thread* mThread;

	/** The server thread */
	HUServerThread* mServerThread;

public:
	/**
	 * Constructor
	 */
	HUServer();

	/**
	 * Destructor
	 */
	~HUServer();

	/**
	 * Start the server
	 */
	void start();

	/**
	 * Stop the server
	 */
	void stop();

	/**
	 * Check if server is running
	 */
	bool isRunning();

	/**
	 * Wait for this thread
	 */
	void join();
};

}

#endif
