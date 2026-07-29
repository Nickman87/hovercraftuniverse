#include "VisualEventParser.h"
#include "CollisionEvent.h"

namespace HovUni {

VisualEventParser::~VisualEventParser(void)
{
}

VisualEvent* VisualEventParser::parse(ZCom_BitStream* stream){
	if ( !VisualEvent::checkEventClass(stream) ){
		return 0;
	}

	VisualEventType type = VisualEvent::readType(stream);

	switch (type) {
	case onCollision:
		return CollisionEvent::parse(stream);
	default:
		return 0;
		break;
	}
}

}
