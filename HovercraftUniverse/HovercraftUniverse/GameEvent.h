#ifndef GAMEEVENT_H_
#define GAMEEVENT_H_

#include "NetworkEvent.h"
#include "GameEventType.h"

namespace HovUni {

/**
 * This is an event during the game. It is serializable so that
 * it can be sent across the network.
 *
 * @author Olivier Berghmans
 */
class GameEvent : public NetworkEvent<GameEventType, GameEventTypeSize,3> {
public:
	/**
	 * Constructor
	 *
	 * @param type the type of the event (used for serialization)
	 */
	GameEvent(GameEventType type);

	/**
	 * Destructor
	 */
	~GameEvent();
};

/*
 * Macros for quickly declaring and implementing exception classes.
 */
#define HOVUNI_GAMEEVENT_DECLARE(NEW, TYPE)									\
	class NEW: public HovUni::GameEvent										\
	{																		\
	public:																	\
		NEW();																\
		~NEW();																\
		void write(ZCom_BitStream* stream) const;							\
		void read(ZCom_BitStream* stream);									\
		static NEW* parse(ZCom_BitStream* stream);							\
	};


#define HOVUNI_GAMEEVENT_DEFINITION(NEW, TYPE)								\
	NEW::NEW() : HovUni::GameEvent(TYPE) { }								\
	NEW::~NEW() { }															\
	void NEW::write(ZCom_BitStream* stream) const { }						\
	void NEW::read(ZCom_BitStream* stream) { }								\
	NEW* NEW::parse(ZCom_BitStream* stream) {						\
		NEW* e = new NEW();													\
		e->deserialize(stream);												\
		return e;															\
	}

/*
 * Simple game events
 */
HOVUNI_GAMEEVENT_DECLARE(StartTrackEvent, startTrack)
}

#endif
