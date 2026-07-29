#ifndef HUDEDICATEDSERVER_H_
#define HUDEDICATEDSERVER_H_

#include "DedicatedServer.h"
#include "HUServer.h"

namespace HovUni {
	/**
	*	The HoverCraftUniverse Dedicated Server Application Class.
	*	@author Dirk Delahaye, 30/03/2010
	*/
	class HUDedicatedServer : DedicatedServer {
	private:
		/** The server that is running */
		HUServer* mServer;

	public:
		/**
		 * Constructor for the dedicated server
		 *
		 * @param configINI The ini to be used.
		 */
		HUDedicatedServer(const std::string& configINI);

		/** Destructor */
		virtual ~HUDedicatedServer();

		/**
		*	See DedicatedServer::init()
		*/
		void init();

		/**
		*	See DedicatedServer::run()
		*/
		void run(bool standalone = true);

		/**
		 * Stop the Dedicated Server
		 */
		void stop();
	};
}
#endif //HUDEDICATEDSERVER_H_