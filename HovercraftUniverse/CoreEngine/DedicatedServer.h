#ifndef DEDICATEDSERVER_H_
#define DEDICATEDSERVER_H_

#include "Config.h"


namespace HovUni {

	/**
	*	Dedicated server class. Similar in function to Application.
	*	TODO:	Check if Some parts of this and Application are 
	*			common enough for abstraction into a parent class.
	*	@author Dirk Delahaye, 30/03/2010
	*/
	class DedicatedServer {
	protected:
		/**
		*	The singleton Engine Settings object.
		*/
		static Config* mEngineSettings;
		
		/**
		*	The config pointer.
		*/
		static Config* mConfig;

		/**
		*	Config File name
		*/
		std::string mConfigFilename;

	public:
		/**
		*	Constructor.
		*	@param	configINI	The Config INI file name.
		*/
		DedicatedServer(const std::string& configINI);

		/**
		*	
		*/
		~DedicatedServer();

		/**
		*	Initialize the Server.
		*/
		void init();

		/**
		*	Parse the configuration file.
		*/
		void parseIni();

		/**
		*	Run the server.
		*/
		void run(bool standalone = false);

		/**
		*	Returns a pointer to the singleton Engine Settings object.
		*	@return	The Config object (singleton).
		*/
		static Config* getEngineSettings();
	
		/**
		*	Returns a pointer to the singleton Config object.
		*	@return	The Config object (singleton).
		*/
		static Config* getConfig();
	};

}

#endif //DEDICATEDSERVER_H_
