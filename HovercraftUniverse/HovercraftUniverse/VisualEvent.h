#ifndef VISUALEVENT_H
#define VISUALEVENT_H

#include "NetworkEvent.h"
#include "VisualEventType.h"

namespace HovUni {

/**
 * This is an event during the game that only has a visibilty effect. It is serializable so that
 * it can be sent across the network.
 *
 * @author Pieter-Jan Pintens
 */
class VisualEvent : public NetworkEvent<VisualEventType, VisualEventTypeSize,4>
{
public:

	/**
	 * Constructor
	 *
	 * @param type the type of the event (used for serialization)
	 */
	VisualEvent(VisualEventType type);

	/**
	 * Destructor
	 */
	~VisualEvent(void);
};

}

#endif