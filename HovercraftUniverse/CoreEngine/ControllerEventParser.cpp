#include "ControllerEventParser.h"
#include "BasicEntityEvent.h"

namespace HovUni {

ControllerEventParser::~ControllerEventParser() {

}

ControllerEvent* ControllerEventParser::parse(ZCom_BitStream* stream) {
	if ( !ControllerEvent::checkEventClass(stream) ){
		return 0;
	}

	ControllerEventType type = ControllerEvent::readType(stream);

	switch (type) {
		case BasicEntity:
			return BasicEntityEvent::parse(stream);
		default:
			return 0;
	}

}

}
