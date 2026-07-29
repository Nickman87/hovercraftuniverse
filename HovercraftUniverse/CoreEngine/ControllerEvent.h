#ifndef CONTROLLEREVENT_H_
#define CONTROLLEREVENT_H_

#include "NetworkEvent.h"
#include "ControllerEventType.h"
#include <OgrePrerequisites.h>

namespace HovUni {

/**
 * This is an event created by the controller. It is serializable so that
 * it can be sent to the other clients (through the server).
 *
 * @author Olivier Berghmans
 */
class ControllerEvent: public NetworkEvent<ControllerEventType, ControllerEventTypeSize,2>
{
public:
	/**
	 * Constructor
	 *
	 * @param type the type of the event (used for serialization)
	 */
	ControllerEvent(ControllerEventType type);

	/**
	 * Destructor
	 */
	~ControllerEvent();

	/**
	 * Write the event
	 *
	 * @param stream the bitstream to append the event
	 */
	virtual void write(ZCom_BitStream* stream) const;

	/**
	 * Read the event
	 *
	 * @param stream the bitstream containing the event
	 */
	virtual void read(ZCom_BitStream* stream);

};

}

#endif
