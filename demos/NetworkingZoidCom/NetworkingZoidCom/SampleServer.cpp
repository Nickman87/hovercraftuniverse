#include "StdAfx.h"
#include "Player.h"
#include "SampleServer.h"
#include "SampleEventParser.h"
#include "Tree.h"

SampleServer::SampleServer() : NetworkServer(10000, 10001) {
	// Register classes
	registerClasses();
}

SampleServer::~SampleServer() {
}

void SampleServer::process() {
	NetworkServer::process();
	for (vector<NetworkEntity*>::iterator it = mEntities.begin(); it != mEntities.end(); ++it) {
		(*it)->processEvents();
	}
}

void SampleServer::registerClasses() {
	Player::registerClass(this);
	Tree::registerClass(this);
}

void SampleServer::addEntity(NetworkEntity* entity) {
	mEntities.push_back(entity);
}

bool SampleServer::ZCom_cbConnectionRequest(ZCom_ConnID id, ZCom_BitStream& request, ZCom_BitStream& reply) {
	cout << "A client requested connection - the new id is [" << id << "]." << endl;
	return true;
}

void SampleServer::ZCom_cbConnectionSpawned(ZCom_ConnID id) {
	cout << "New connection with id [" << id << "]" << endl;
	Player* player = new Player(this);
	player->getNetworkNode()->setOwner(id, true);
	mEntities.push_back(player);
}

void SampleServer::ZCom_cbConnectionClosed(ZCom_ConnID id, eZCom_CloseReason reason, ZCom_BitStream& reasondata) {
	cout << "Connection with id [" << id << "] closed" << endl;
}

void SampleServer::ZCom_cbDataReceived(ZCom_ConnID id, ZCom_BitStream& data) {
	cout << "Received a string: " << data.getStringStatic() << endl;
}

bool SampleServer::ZCom_cbZoidRequest(ZCom_ConnID id, zU8 requested_level, ZCom_BitStream& reason) {
	if (requested_level == 1) {
		return true;
	} else {
		return false;
	}
}

void SampleServer::ZCom_cbZoidResult(ZCom_ConnID id, eZCom_ZoidResult result, zU8 new_level, ZCom_BitStream& reason) {
	if (result == eZCom_ZoidEnabled) {
		cout << "Zoidlevel " << (int) new_level << " entered" << endl;
	} else {
		cout << "Failed entering Zoidlevel " << new_level << endl;
	}

}
