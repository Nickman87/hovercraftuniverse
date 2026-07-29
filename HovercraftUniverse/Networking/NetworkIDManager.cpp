#include "Exception.h"
#include "NetworkIDManager.h"
#include <cassert>
#include <sstream>

namespace HovUni {

NetworkIDManager* NetworkIDManager::msNetworkIDManager = 0;

map<ZCom_ConnID, NetworkIDManager*> NetworkIDManager::msSingletons;

NetworkIDManager::NetworkIDManager(ZCom_Control* control) :
	mControl(control) {

}

NetworkIDManager::NetworkIDManager() {

}

NetworkIDManager::~NetworkIDManager() {

}

void NetworkIDManager::setControl(ZCom_Control* control) {
	mControl = control;
}

ZCom_Control* NetworkIDManager::getControl() {
	return mControl;
}

ZCom_ClassID NetworkIDManager::registerClass(const string& className, bool announce) {
	assert(mControl);
	ZCom_ClassID classID;
	if (announce) {
		classID = mControl->ZCom_registerClass(className.c_str(), ZCOM_CLASSFLAG_ANNOUNCEDATA);
	} else {
		classID = mControl->ZCom_registerClass(className.c_str());
	}
	mMap.insert(pair<string, ZCom_ClassID> (className, classID));
	return classID;
}

ZCom_ClassID NetworkIDManager::getID(const string& className) const {
	map_type::const_iterator it = mMap.find(className);
	if (it != mMap.end()) {
		return it->second;
	}
	THROW(NetworkException, "No such class name registered");
}

string NetworkIDManager::getName(const ZCom_ClassID& id) const {
	for (map_type::const_iterator it = mMap.begin(); it != mMap.end(); ++it) {
		if (it->second == id) {
			return it->first;
		}
	}
	return "";
}

string NetworkIDManager::info() const {
	std::stringstream ss;
	ss << "Table: " << std::endl;
	for (map_type::const_iterator it = mMap.begin(); it != mMap.end(); ++it) {
		ss << " - " << it->first << " = " << it->second << std::endl;
	}
	return ss.str();
}

NetworkIDManager* NetworkIDManager::getServerSingletonPtr() {
	if (!msNetworkIDManager) {
		msNetworkIDManager = new NetworkIDManager();
	}

	return msNetworkIDManager;
}

NetworkIDManager* NetworkIDManager::getClientSingletonPtr(ZCom_ConnID id) {
	if (msSingletons.find(id) == msSingletons.end()) {
		msSingletons[id] = new NetworkIDManager();
	}

	return msSingletons[id];
}

}
