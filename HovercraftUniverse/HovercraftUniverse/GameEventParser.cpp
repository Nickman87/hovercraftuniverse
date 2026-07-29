#include "GameEventParser.h"

#include "StateEvent.h"
#include "InitEvent.h"
#include "CheckpointEvent.h"

namespace HovUni {

GameEventParser::~GameEventParser() {

}

GameEvent* GameEventParser::parse(ZCom_BitStream* stream) {
	if ( !GameEvent::checkEventClass(stream) ){
		return 0;
	}

	GameEventType type = GameEvent::readType(stream);

	switch (type) {
	case init:
		return InitEvent::parse(stream);
	case stateChange:
		return StateEvent::parse(stream);
	case startTrack:
		return StartTrackEvent::parse(stream);
	case checkPoint:
		return CheckpointEvent::parse(stream);
	default:
		return 0;
	}
}

}
