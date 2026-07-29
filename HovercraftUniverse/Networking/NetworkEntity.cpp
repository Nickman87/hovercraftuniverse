#include "NetworkEntity.h"

#include "OgreVector3_Replicator.h"
#include "String_Replicator.h"
#include "OgreQuaternion_Replicator.h"

namespace HovUni {

NetworkEntity::NetworkEntity(unsigned short replicators) :
	mNode(0), mReplicatorNr(replicators), mRegistered(false), mDeleted(false) {
	mNode = new ZCom_Node();
}

NetworkEntity::~NetworkEntity() {
	if (mNode) {
		delete mNode;
		mNode = 0;
	}
}

void NetworkEntity::networkRegister(ZCom_ClassID id, ZCom_Control* control, bool announce) {
	// Set up replication
	mNode->beginReplicationSetup(mReplicatorNr);
	setupReplication();
	mNode->endReplicationSetup();

	// Register to node
	mNode->registerNodeDynamic(id, control);
	mRegistered = true;

	// Check to set announcement data
	if (announce) {
		ZCom_BitStream* stream = new ZCom_BitStream();
		setAnnouncementData(stream);
		mNode->setAnnounceData(stream);
	}
}

void NetworkEntity::networkRegister(NetworkIDManager* idmanager, std::string name, bool announce) {
	networkRegister(idmanager->getID(name), idmanager->getControl(), announce);
}

void NetworkEntity::networkRegisterUnique(ZCom_ClassID id, ZCom_Control* control, bool authority) {
	// Set up replication
	mNode->beginReplicationSetup(mReplicatorNr);
	setupReplication();
	mNode->endReplicationSetup();

	// Register the object
	eZCom_NodeRole role = (authority) ? eZCom_RoleAuthority : eZCom_RoleProxy;
	mNode->registerNodeUnique(id, role, control);
	mRegistered = true;
}

void NetworkEntity::networkRegisterUnique(NetworkIDManager* idmanager, std::string name,
		bool authority) {
	networkRegisterUnique(idmanager->getID(name), idmanager->getControl());
}

bool NetworkEntity::isRegistered() const {
	return mRegistered;
}

bool NetworkEntity::isDeleted() const {
	return mDeleted;
}

void NetworkEntity::processEvents(float timeSince) {
	while (mNode->checkEventWaiting()) {
		eZCom_Event type;
		eZCom_NodeRole remote_role;
		ZCom_ConnID conn_id;

		ZCom_BitStream* data = mNode->getNextEvent(&type, &remote_role, &conn_id);

		if (remote_role == eZCom_RoleAuthority && type == eZCom_EventRemoved) {
			mDeleted = true;
		}

		// Callback to parse the events
		parseEvents(type, remote_role, conn_id, data, timeSince);
	}
}

ZCom_Node* NetworkEntity::getNetworkNode() {
	return mNode;
}

void NetworkEntity::setReplicationInterceptor(ZCom_NodeReplicationInterceptor* interceptor) {
	mNode->setReplicationInterceptor(interceptor);

}

void NetworkEntity::replicateUnsignedInt(int* value, zU8 rules, zU8 mantissaBits, zU8 flags,
		zS16 minDelay, zS16 maxDelay) {
	mNode->addReplicationInt(value, mantissaBits, false, flags, rules, minDelay, maxDelay);
}

void NetworkEntity::replicateInterpolationFloat(float* value, float margin, zU8 rules, zU8 mantissaBits, zU8 flags,
		zS16 minDelay, zS16 maxDelay, float ipolfac, float* tmp) {
	mNode->addInterpolationFloat(value, mantissaBits, flags, rules, margin, tmp, minDelay, maxDelay, ipolfac);
}

void NetworkEntity::replicateFloat(float* value, zU8 rules, zU8 mantissaBits, zU8 flags,
		zS16 minDelay, zS16 maxDelay) {
	mNode->addReplicationFloat(value, mantissaBits, flags, rules, minDelay, maxDelay);
	
}

void NetworkEntity::replicateOgreVector3(Ogre::Vector3* vector, zU8 rules, zU8 mantissaBits,
		zU8 flags, zU8 interceptID, zS16 minDelay, zS16 maxDelay, bool autoDelete) {
	mNode->addReplicator(new OgreVector3_Replicator(vector, mantissaBits, flags, rules,
			interceptID, minDelay, maxDelay), autoDelete);
}

void NetworkEntity::replicateOgreQuaternion(Ogre::Quaternion* quat, zU8 rules, zU8 mantissaBits,
		zU8 flags, zU8 interceptID, zS16 minDelay, zS16 maxDelay, bool autoDelete) {
	mNode->addReplicator(new OgreQuaternion_Replicator(quat, mantissaBits, flags, rules,
			interceptID, minDelay, maxDelay), autoDelete);
}

void NetworkEntity::replicateString(Ogre::String * str, zU8 rules, zU8 flags, zU8 interceptID,
		zS16 minDelay, zS16 maxDelay, bool autoDelete) {
	mNode->addReplicator(new String_Replicator(str, flags, rules, interceptID, minDelay, maxDelay),
			autoDelete);
}

}
