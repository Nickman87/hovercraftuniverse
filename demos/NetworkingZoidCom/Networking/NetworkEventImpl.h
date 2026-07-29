namespace HovUni {

template <typename EventType>
NetworkEvent<EventType>::NetworkEvent(EventType type) : mType(type) {

}

template <typename EventType>
NetworkEvent<EventType>::~NetworkEvent() {

}

template <typename EventType>
EventType NetworkEvent<EventType>::getType() const {
	return mType;
}

template <typename EventType>
void NetworkEvent<EventType>::serialize(ZCom_BitStream* stream) const {
	// Write the type of the event
	stream->addInt((int) mType, 4);

	// Write the data of the event
	write(stream);
}

template <typename EventType>
void NetworkEvent<EventType>::deserialize(ZCom_BitStream* stream) {
	// Read the type of the event
	mType = (EventType) (int) stream->getInt(4);

	// Read the data of the event
	read(stream);
}

template <typename EventType>
EventType NetworkEvent<EventType>::readType(ZCom_BitStream* stream) {
	ZCom_BitStream::BitPos pos;
	stream->saveReadState(pos);
	EventType t = (EventType) (int) stream->getInt(4);
	stream->restoreReadState(pos);
	return t;
}

}
