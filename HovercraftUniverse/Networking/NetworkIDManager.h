#ifndef NETWORKIDMANAGER_H_
#define NETWORKIDMANAGER_H_

#include <map>
#include <string>
#include <zoidcom/zoidcom_control.h>

using std::map;
using std::string;
using std::pair;

namespace HovUni {

/**
 * This class holds the class to ID mapping for the network framework.
 *
 * @author Olivier Berghmans
 */
class NetworkIDManager {
public:
	/**
	 * Returns the id manager singleton for the server.
	 *
	 * @return the singleton pointer
	 */
	static NetworkIDManager* getServerSingletonPtr();

	/**
	 * Returns the id manager singleton for a client.
	 *
	 * @param id the clients connection ID
	 * @return the singleton pointer
	 */
	static NetworkIDManager* getClientSingletonPtr(ZCom_ConnID id);

private:
	/** The server singleton */
	static NetworkIDManager* msNetworkIDManager;

	/** The static map of singletons */
	static map<ZCom_ConnID, NetworkIDManager*> msSingletons;

	/** The map type */
	typedef map<string, ZCom_ClassID> map_type;

	/** The map to ID */
	map_type mMap;

	/** The control */
	ZCom_Control* mControl;

public:
	/**
	 * Constructor
	 *
	 * @param control the network control
	 */
	NetworkIDManager(ZCom_Control* control);

	/**
	 * Destructor
	 */
	~NetworkIDManager();

	/**
	 * Set the network control in order to register
	 * classes
	 * 
	 * @param control the network control
	 */
	void setControl(ZCom_Control* control);

	/**
	 * Get the network control
	 * 
	 * @return the network control
	 */
	ZCom_Control* getControl();

	/**
	 * Register a class. A control must be set in order for registration
	 * to succeed.
	 *
	 * @param className the name of the class
	 * @param announce whether this class will have announce data
	 * @return the ID
	 */
	ZCom_ClassID registerClass(const string& className, bool announce = false);

	/**
	 * Get an ID for a class
	 *
	 * @param className the name of the class
	 * @return the ID
	 */
	ZCom_ClassID getID(const string& className) const;

	/**
	 * Get the class for an ID (probably used for debugging)
	 *
	 * @param id the ID
	 * @return the name of the class
	 */
	string getName(const ZCom_ClassID& id) const;

	/**
	 * Get the info
	 *
	 * @return the info
	 */
	string info() const;

private:
	/**
	 * Constructor
	 */
	NetworkIDManager();

	/**
	 * Hide the copy constructor
	 */
	NetworkIDManager(const NetworkIDManager&) {
	}

};

}

#endif
