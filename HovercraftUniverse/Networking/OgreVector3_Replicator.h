#ifndef OGREVECTOR3_REPLICATOR_H
#define OGREVECTOR3_REPLICATOR_H

#include <OgreVector3.h>
#include <zoidcom/zoidcom.h>

namespace HovUni {

/**
 * A class that implements a zoidcom replicator object to be able to
 * replicate Ogre::Vector3 objects with ease.
 *
 * @author Nick De Frangh
 */
class OgreVector3_Replicator: public ZCom_ReplicatorBasic {
private:
	/** The real data to be synchronised */
	Ogre::Vector3* mData;

	/** A replica to discover changes */
	Ogre::Vector3 mCompare;

	/** The number of mantissa bits you want to synchronise */
	zU8 mMantissaBits;

public:
	/**
	 * Constructor
	 *
	 * @param data the pointer to the data
	 * @param _setup the replicator setup
	 * @param mantissa the amount of mantissa bits to replicate
	 */
	OgreVector3_Replicator(Ogre::Vector3 *data, ZCom_ReplicatorSetup *_setup, zU8 mantissa = 10);

	/**
	 * Constructor
	 *
	 * @param data the pointer to the data
	 * @param mantissa the amount of mantissa bits to replicate
	 * @param _flags the flags on how to replicate
	 * @param _rules the rules in which direction the replication works
	 * @param _intercept_id the interceptor ID for this replicator
	 * @param _mindelay the minimum delay for the replication
	 * @param _maxdeley the maximum delay for the replication
	 */
	OgreVector3_Replicator(Ogre::Vector3 *data, zU8 mantissa, zU8 _flags, zU8 _rules,
			zU8 _intercept_id = 0, zS16 _mindelay = -1, zS16 _maxdelay = -1);

	/**
	 * Destructor
	 */
	virtual ~OgreVector3_Replicator();

	/**
	 * Check the state of the variable to replicate
	 *
	 * @return true if there needs to be an update
	 */
	bool checkState();

	/**
	 * Pack the data in the stream in order to send an update
	 *
	 * @param _stream the stream
	 */
	void packData(ZCom_BitStream *_stream);

	/**
	 * Unpack the data from a stream after receiving an update
	 *
	 * @param _stream the stream
	 * @param _store whether to store the new data
	 * @param _estimated_time_sent indicator of when the data was approximately sent
	 */
	void unpackData(ZCom_BitStream *_stream, bool _store, zU32 _estimated_time_sent);

	/**
	 * Peek at the data without actually unpacking it
	 */
	void* peekData();

	/**
	 * Clear data used when peeking at data
	 */
	void clearPeekData();

	/**
	 * Process
	 *
	 * @param _localrole the local node role
	 * @param _simulation_time_passed the time passed in the simulation
	 */
	void Process(eZCom_NodeRole _localrole, zU32 _simulation_time_passed);
};
}

#endif //OGREVECTOR3_REPLICATOR_H
