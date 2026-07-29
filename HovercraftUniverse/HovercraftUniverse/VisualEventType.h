#ifndef VISUALEVENTTYPE_H_
#define VISUALEVENTTYPE_H_

namespace HovUni {

/**
 * This is an event type for controller events.
 *
 * @author Pieter-Jan Pintens
 */
enum VisualEventType {
	onCollision
	// Make sure to adjust the amount below
};

/** The amount of controller event types, used for compressing in the network */
static const unsigned VisualEventTypeSize = 1;

}

#endif
