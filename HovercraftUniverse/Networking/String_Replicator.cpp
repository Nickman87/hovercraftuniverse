#include "String_Replicator.h"

#include "Ogre.h"

namespace HovUni {

String_Replicator::String_Replicator(Ogre::String *data, ZCom_ReplicatorSetup *_setup) :
	ZCom_ReplicatorBasic(_setup), mData(data), mCompare(*data) {

	m_flags |= ZCOM_REPLICATOR_INITIALIZED;
}

String_Replicator::String_Replicator(Ogre::String *data, zU8 _flags, zU8 _rules, zU8 _intercept_id,
		zS16 _mindelay, zS16 _maxdelay) :
	ZCom_ReplicatorBasic(NULL), mData(data), mCompare(*data) {
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

String_Replicator::~String_Replicator() {
	//No cleanup needed
}

bool String_Replicator::checkState() {
	// compare
	bool same = ((*mData) == mCompare);
	// if changed, update the comparison
	if (!same) {
		mCompare = (*mData);
	}
	// return true if an update is needed
	return !same;
}

void String_Replicator::packData(ZCom_BitStream *_stream) {
	// add data to stream
	_stream->addInt(mData->length(), sizeof(zU16) * 8);
	if (!mData->empty()) {
		_stream->addBuffer(const_cast<char *> (mData->c_str()), mData->length());
	}
}

void String_Replicator::unpackData(ZCom_BitStream *_stream, bool _store, zU32 _estimated_time_sent) {
	// shall we store
	if (_store) {

		zU32 bytes = _stream->getInt(sizeof(zU16) * 8);

		if (bytes == 0) {
			//if empty
			*mData = "";
		} else if (bytes <= BUFFERSIZE) {
			//if internal buffer big enough
			_stream->getBuffer(mBuffer, bytes);
			*mData = Ogre::String(mBuffer, bytes);
		} else {
			//if real big text, fetch all
			*mData = "";
			zU32 bytesleft = bytes;
			while (bytesleft > 0) {
				if (bytesleft > BUFFERSIZE) {
					_stream->getBuffer(mBuffer, BUFFERSIZE);
					*mData += Ogre::String(mBuffer, BUFFERSIZE);
				} else {
					_stream->getBuffer(mBuffer, bytesleft);
					*mData += Ogre::String(mBuffer, bytesleft);
				}

				bytesleft -= BUFFERSIZE;
			}
		}
		// or just skip the data in the stream?
	} else {
		zU16 bytes = _stream->getInt(sizeof(zU16) * 8);
		if (bytes != 0) {
			_stream->skipBuffer(bytes);
		}
	}
}

void* String_Replicator::peekData() {
	// check if peekData is really called from within an interceptor
	assert(getPeekStream());

	// read data into the buffer
	zU16 bytes = getPeekStream()->getInt(sizeof(zU16) * 8);

	// allocate memory as peekbuffer
	char *buf = new char[bytes];
	if (!buf) {
		return NULL;
	}

	getPeekStream()->getBuffer(buf, bytes);

	// let zoidcom store the pointer to the buffer so we can delete it later
	peekDataStore(buf);
	// return the buffer so the caller can look at the data
	return buf;
}

void String_Replicator::clearPeekData() {
	// get the data we allocated above
	char *buf = (char*) peekDataRetrieve();
	// and delete it
	if (buf)
		delete[] buf;
}

void String_Replicator::Process(eZCom_NodeRole _localrole, zU32 _simulation_time_passed) {
	//I don't get it? :)
}
}
