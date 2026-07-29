#include "OgreVector3_Replicator.h"

namespace HovUni {

OgreVector3_Replicator::OgreVector3_Replicator(Ogre::Vector3 *data, ZCom_ReplicatorSetup *_setup,
		zU8 mantissa) :
	ZCom_ReplicatorBasic(_setup), mData(data), mCompare(*data), mMantissaBits(mantissa) {

	m_flags |= ZCOM_REPLICATOR_INITIALIZED;
}

OgreVector3_Replicator::OgreVector3_Replicator(Ogre::Vector3 *data, zU8 mantissa, zU8 _flags,
		zU8 _rules, zU8 _intercept_id, zS16 _mindelay, zS16 _maxdelay) :
	ZCom_ReplicatorBasic(NULL), mData(data), mCompare(*data), mMantissaBits(mantissa) {
	// make sure m_setup will be deleted when the replicator is deleted
	_flags |= ZCOM_REPFLAG_SETUPAUTODELETE;

	// create the setup object
	m_setup = new ZCom_ReplicatorSetup(_flags, _rules, _intercept_id, _mindelay, _maxdelay);

	// out of mem
	if (!m_setup) {
		return;
	}

	m_flags |= ZCOM_REPLICATOR_INITIALIZED;
}

OgreVector3_Replicator::~OgreVector3_Replicator() {
	//No cleanup needed
}

bool OgreVector3_Replicator::checkState() {
	// compare
	bool same = ((*mData) == mCompare);
	// if changed, update the comparison
	if (!same) {
		mCompare = (*mData);
	}
	// return true if an update is needed
	return !same;
}

void OgreVector3_Replicator::packData(ZCom_BitStream *_stream) {
	// add data to stream
	_stream->addFloat(mData->x, mMantissaBits);
	_stream->addFloat(mData->y, mMantissaBits);
	_stream->addFloat(mData->z, mMantissaBits);
}

void OgreVector3_Replicator::unpackData(ZCom_BitStream *_stream, bool _store,
		zU32 _estimated_time_sent) {
	// shall we store
	if (_store) {
		mData->x = _stream->getFloat(mMantissaBits);
		mData->y = _stream->getFloat(mMantissaBits);
		mData->z = _stream->getFloat(mMantissaBits);
		// or just skip the data in the stream?
	} else {
		_stream->skipFloat(mMantissaBits);
		_stream->skipFloat(mMantissaBits);
		_stream->skipFloat(mMantissaBits);
	}
}

void* OgreVector3_Replicator::peekData() {
	// check if peekData is really called from within an interceptor
	assert(getPeekStream());

	// read data into the buffer
	zFloat x = getPeekStream()->getFloat(mMantissaBits);
	zFloat y = getPeekStream()->getFloat(mMantissaBits);
	zFloat z = getPeekStream()->getFloat(mMantissaBits);

	// allocate memory as peekbuffer
	char *buf = new char[sizeof(x) + sizeof(y) + sizeof(z)];
	if (!buf) {
		return NULL;
	}

	//Store the floats in the buffer
	memcpy(buf, &x, sizeof(x));
	memcpy(buf + sizeof(x), &y, sizeof(y));
	memcpy(buf + sizeof(x) + sizeof(y), &z, sizeof(z));

	// let zoidcom store the pointer to the buffer so we can delete it later
	peekDataStore(buf);
	// return the buffer so the caller can look at the data
	return buf;
}

void OgreVector3_Replicator::clearPeekData() {
	// get the data we allocated above
	char *buf = (char*) peekDataRetrieve();
	// and delete it
	if (buf)
		delete[] buf;
}

void OgreVector3_Replicator::Process(eZCom_NodeRole _localrole, zU32 _simulation_time_passed) {
	//I don't get it? :)
}

}
