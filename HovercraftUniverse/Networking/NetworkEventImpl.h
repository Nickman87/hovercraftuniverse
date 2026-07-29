#include <math.h>

namespace HovUni {

template<unsigned N>
unsigned int logn(unsigned n) {
	return (unsigned int) ceil(log((double) n) / log((double) N));
}

template<typename EventType, unsigned N, int EventClass>
NetworkEvent<EventType, N, EventClass>::NetworkEvent(EventType type) :
	mType(type) {

}

template<typename EventType, unsigned N, int EventClass>
NetworkEvent<EventType, N, EventClass>::~NetworkEvent() {

}

template<typename EventType, unsigned N, int EventClass>
EventType NetworkEvent<EventType, N, EventClass>::getType() const {
	return mType;
}

template<typename EventType, unsigned N, int EventClass>
void NetworkEvent<EventType, N, EventClass>::serialize(ZCom_BitStream* stream) const {
	//write class of the event
	stream->addInt(EventClass,5);

	// Write the type of the event
	if (N != 0) {
		stream->addInt((int) mType, logn<2> (N));
	} else {
		stream->addInt((int) mType, 1);
	}

	// Write the data of the event
	write(stream);
}

template<typename EventType, unsigned N, int EventClass>
void NetworkEvent<EventType, N, EventClass>::deserialize(ZCom_BitStream* stream) {
	//ignore the type
	stream->skipInt(5);

	// Read the type of the event
	if (N != 0) {
		mType = (EventType) (int) stream->getInt(logn<2> (N));
	} else {
		mType = (EventType) (int) stream->getInt(1);
	}

	// Read the data of the event
	read(stream);
}


template<typename EventType, unsigned N, int EventClass>
bool NetworkEvent<EventType, N, EventClass>::checkEventClass(ZCom_BitStream* stream){
	ZCom_BitStream::BitPos pos;
	stream->saveReadState(pos);
	int eventclass = (int) stream->getInt(5);
	stream->restoreReadState(pos);
	return eventclass == EventClass;	//return true if eventclass is the same
}

template<typename EventType, unsigned N, int EventClass>
EventType NetworkEvent<EventType, N, EventClass>::readType(ZCom_BitStream* stream) {
	ZCom_BitStream::BitPos pos;
	stream->saveReadState(pos);

	//ignore the type
	stream->skipInt(5);

	EventType t;
	if (N != 0) {
		t = (EventType) (int) stream->getInt(logn<2> (N));
	} else {
		t = (EventType) (int) stream->getInt(1);
	}
	stream->restoreReadState(pos);
	return t;
}

}
