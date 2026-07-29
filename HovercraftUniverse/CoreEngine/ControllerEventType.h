#ifndef CONTROLLEREVENTTYPE_H_
#define CONTROLLEREVENTTYPE_H_

namespace HovUni {

/**
 * This is an event type for controller events.
 *
 * @author Olivier Berghmans
 */
enum ControllerEventType {
	BasicEntity
	// Make sure to adjust the amount below
};

/** The amount of controller event types, used for compressing in the network */
static const unsigned ControllerEventTypeSize = 1;

}

#endif
