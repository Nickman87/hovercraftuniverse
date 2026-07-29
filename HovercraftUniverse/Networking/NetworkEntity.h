#ifndef NETWORKENTITY_H_
#define NETWORKENTITY_H_

#define ZCOM_REPFLAG_SETUPAUTODELETE   (1L << 6)

#include "NetworkEvent.h"
#include "NetworkIDManager.h"
#include <OgreVector3.h>
#include <zoidcom/zoidcom_node.h>
#include <string>

namespace HovUni {

/**
 * This class represents a network entity.
 *
 * @author Olivier Berghmans
 */
class NetworkEntity {
protected:
	/** The network node */
	ZCom_Node* mNode;

	/** The estimated number of replicators */
	unsigned short mReplicatorNr;

	/** Whether the entity is registered to the network */
	bool mRegistered;

	/** Whether the entity is deleted on the network */
	bool mDeleted;

public:
	/**
	 * Constructor
	 *
	 * @param replicators the estimated number of replicators for this entity
	 */
	NetworkEntity(unsigned short replicators);

	/**
	 * Destructor
	 */
	virtual ~NetworkEntity();

	/**
	 * Register the node for the network
	 *
	 * @param id the zoidcom class ID
	 * @param control the zoidcom control
	 * @param announce whether the announcement data should be set (Use this on server only)
	 */
	void networkRegister(ZCom_ClassID id, ZCom_Control* control, bool announce = false);

	/**
	 * Register the node for the network
	 *
	 * @param idmanager the network ID manager
	 * @param name the registered class name
	 * @param announce whether the announcement data should be set (Use this on server only)
	 */
	void networkRegister(NetworkIDManager* idmanager, std::string name, bool announce = false);

	/**
	 * Register a singleton object on the network.
	 *
	 * @param id the zoidcom class ID
	 * @param control the zoidcom control
	 * @param authority whether we are the authority of this object or not (server only)
	 */
	void networkRegisterUnique(ZCom_ClassID id, ZCom_Control* control, bool authority = false);

	/**
	 * Register a singleton object on the network.
	 *
	 * @param idmanager the network ID manager
	 * @param name the registered class name
	 * @param authority whether we are the authority of this object or not (server only)
	 */
	void networkRegisterUnique(NetworkIDManager* idmanager, std::string name, bool authority =
			false);

	/**
	 * Check whether the entity is registered for the network
	 *
	 * @return true if the entity is registered
	 */
	bool isRegistered() const;

	/**
	 * Check whether the entity is deleted on the network
	 *
	 * @return true if the entity was deleted
	 */
	bool isDeleted() const;

	/**
	 * Process the events waiting for this entity. This method
	 * should be called regularly because otherwise events for
	 * this entity will stay in the queue.
	 *
	 * @param timeSince the time since the last process of the events
	 */
	void processEvents(float timeSince);

	/**
	 * Send an event for this entity
	 *
	 * @param event the network event
	 * @param secret false if the data may be send to all proxies
	 * @return result of the send operation
	 */
	template<typename EventType, int N, int EventClass>
	bool sendEvent(const NetworkEvent<EventType, N, EventClass>& nEvent, bool secret = false) const;

	/**
	 * Send an event for this entity
	 *
	 * @param event the network event
	 * @param id the ID to send to
	 * @return result of the send operation
	 */
	template<typename EventType, int N, int EventClass>
	bool sendEventDirect(const NetworkEvent<EventType, N, EventClass>& nEvent, ZCom_ConnID id) const;

	/**
	 * Get the network node. You should probably not be calling this.
	 *
	 * @return the network node
	 */
	ZCom_Node* getNetworkNode();

	/**
	 * Add a replicator interceptor
	 *
	 * @param interceptor The interceptor to add
	 */
	void setReplicationInterceptor(ZCom_NodeReplicationInterceptor* interceptor);

private:
	/**
	 * Hide the copy constructor
	 */
	NetworkEntity(const NetworkEntity&) :
		mNode(0), mRegistered(false) {
	}

	/**
	 * A callback that should be implemented so that events for
	 * this entity are handled in a good way. Normally this method
	 * would first parse the stream to an event and then process
	 * the event.
	 *
	 * @param type off event
	 * @param role of remote
	 * @param connection id of remote
	 * @param stream the bitstream containing the event
	 * @param timeSince the time since the last processing of the events
	 */
	virtual void parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id,
			ZCom_BitStream* stream, float timeSince) = 0;

protected:

	/**
	 * A callback that should be implemented so the replicators for this
	 * entity can be set up. In case the number of replicators given to the
	 * constructor is zero, this callback can be implemented as empty.
	 */
	virtual void setupReplication() = 0;

	/**
	 * A callback that should be implemented so the announcement data for this
	 * entity can be set.
	 *
	 * @param stream the bitstream where the data can be set
	 */
	virtual void setAnnouncementData(ZCom_BitStream* stream) = 0;

	/**
	 * Add a int to the replicated variables
	 *
	 * @param value The int to replicate
	 * @param rules The rules for the replicator, this indicates whether the
	 *				variable is replicated from the server only, or whether the
	 *				owner can change the value too
	 * @param mantissaBits the number of mantissa bits to replicate, default is 23
	 * @param flags flags to indicate how the value must be replicated, default is ZCOM_REPFLAG_MOSTRECENT
	 * @param minDelay the minimum delay between to updates, default is -1 (no minimum)
	 * @param maxDelay the maximum delay between to updates, default is -1 (no maximum)
	 */
	void replicateUnsignedInt(int* value, zU8 rules, zU8 mantissaBits = 23, zU8 flags =
			ZCOM_REPFLAG_MOSTRECENT, zS16 minDelay = -1, zS16 maxDelay = -1);

	/**
	 * Add a float to the replicated variables and make it interpolate
	 *
	 * @param value The float to replicate
	 * @param margin The error margin. If the difference between the received and the current value is bigger than _treshold,
     *			Zoidcom will just set current = received without interpolation.
	 * @param rules The rules for the replicator, this indicates whether the
	 *				variable is replicated from the server only, or whether the
	 *				owner can change the value too
	 * @param mantissaBits the number of mantissa bits to replicate, default is 23
	 * @param flags flags to indicate how the value must be replicated, default is ZCOM_REPFLAG_MOSTRECENT
	 * @param minDelay the minimum delay between to updates, default is -1 (no minimum)
	 * @param maxDelay the maximum delay between to updates, default is -1 (no maximum)
	 * @param ipolfac The interpolation factor, the higher it becomes, the higher the influence of the incoming data over the local data.
     *                Set to 0 to ignore incoming data, or 1 to just write incoming data to value. (default 0.4f)
	 * @param tmp If set, Zoidcom will store incoming updates in tmp and interpolate against tmp. If not set, Zoidcom will use internal memory.
	 */
	void replicateInterpolationFloat(float* value, float margin, zU8 rules, zU8 mantissaBits, zU8 flags,
		zS16 minDelay, zS16 maxDelay, float ipolfac, float* tmp);

	/**
	 * Add a float to the replicated variables
	 *
	 * @param value The float to replicate
	 * @param rules The rules for the replicator, this indicates whether the
	 *				variable is replicated from the server only, or whether the
	 *				owner can change the value too
	 * @param mantissaBits the number of mantissa bits to replicate, default is 23
	 * @param flags flags to indicate how the value must be replicated, default is ZCOM_REPFLAG_MOSTRECENT
	 * @param minDelay the minimum delay between to updates, default is -1 (no minimum)
	 * @param maxDelay the maximum delay between to updates, default is -1 (no maximum)
	 */
	void replicateFloat(float* value, zU8 rules, zU8 mantissaBits = 23, zU8 flags =
			ZCOM_REPFLAG_MOSTRECENT, zS16 minDelay = -1, zS16 maxDelay = -1);

	/**
	 * Add a vector to the replicated variables
	 *
	 * @param vector The vector to replicate
	 * @param rules The rules for the replicator, this indicates whether the
	 *				variable is replicated from the server only, or whether the
	 *				owner can change the value too
	 * @param mantissaBits the number of mantissa bits to replicate, default is 23
	 * @param flags flags to indicate how the value must be replicated, default is ZCOM_REPFLAG_MOSTRECENT
	 * @param interceptID the ID for this replicated so it can be checked in the interceptor, default is 0
	 * @param minDelay the minimum delay between to updates, default is -1 (no minimum)
	 * @param maxDelay the maximum delay between to updates, default is -1 (no maximum)
	 * @param autodelete whether to autodelete, default is true
	 */
	void replicateOgreVector3(Ogre::Vector3* vector, zU8 rules, zU8 mantissaBits = 23, zU8 flags =
			ZCOM_REPFLAG_MOSTRECENT, zU8 interceptID = 0, zS16 minDelay = -1, zS16 maxDelay = -1,
			bool autoDelete = true);

	/**
	 * Add a quaternion to the replicated variables
	 *
	 * @param qut the quaternion to replicate
	 * @param rules The rules for the replicator, this indicates whether the
	 *				variable is replicated from the server only, or whether the
	 *				owner can change the value too
	 * @param mantissaBits the number of mantissa bits to replicate, default is 23
	 * @param flags flags to indicate how the value must be replicated, default is ZCOM_REPFLAG_MOSTRECENT
	 * @param interceptID the ID for this replicated so it can be checked in the interceptor, default is 0
	 * @param minDelay the minimum delay between to updates, default is -1 (no minimum)
	 * @param maxDelay the maximum delay between to updates, default is -1 (no maximum)
	 * @param autodelete whether to autodelete, default is true
	 */
	void replicateOgreQuaternion(Ogre::Quaternion* quat, zU8 rules, zU8 mantissaBits = 23,
			zU8 flags = ZCOM_REPFLAG_MOSTRECENT, zU8 interceptID = 0, zS16 minDelay = -1,
			zS16 maxDelay = -1, bool autoDelete = true);

	/**
	 * Add a string to the replicated variables
	 *
	 * @param str the string to replicate
	 * @param rules The rules for the replicator, this indicates whether the
	 *				variable is replicated from the server only, or whether the
	 *				owner can change the value too
	 * @param flags flags to indicate how the value must be replicated, default is ZCOM_REPFLAG_MOSTRECENT
	 * @param interceptID the ID for this replicated so it can be checked in the interceptor, default is 0
	 * @param minDelay the minimum delay between to updates, default is -1 (no minimum)
	 * @param maxDelay the maximum delay between to updates, default is -1 (no maximum)
	 * @param autodelete whether to autodelete, default is true
	 */
	void replicateString(Ogre::String * str, zU8 rules, zU8 flags = ZCOM_REPFLAG_MOSTRECENT,
			zU8 interceptID = 0, zS16 minDelay = -1, zS16 maxDelay = -1, bool autoDelete = true);

};

template<typename EventType, int N, int EventClass>
bool NetworkEntity::sendEvent(const NetworkEvent<EventType, N, EventClass>& nEvent, bool secret) const {
	ZCom_BitStream* stream = new ZCom_BitStream();
	nEvent.serialize(stream);

	eZCom_NodeRole role = mNode->getRole();
	if (role == eZCom_RoleAuthority) {
		if (secret) {
			return mNode->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_OWNER, stream);
		} else {
			return mNode->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_AUTH_2_ALL, stream);
		}
	} else if (role == eZCom_RoleOwner) {
		return mNode->sendEvent(eZCom_ReliableOrdered, ZCOM_REPRULE_OWNER_2_AUTH, stream);
	}
	return false;
}

template<typename EventType, int N, int EventClass>
bool NetworkEntity::sendEventDirect(const NetworkEvent<EventType, N, EventClass>& nEvent, ZCom_ConnID id) const {
	ZCom_BitStream* stream = new ZCom_BitStream();
	nEvent.serialize(stream);
	return mNode->sendEventDirect(eZCom_ReliableOrdered, stream, id);
}

}

#endif
