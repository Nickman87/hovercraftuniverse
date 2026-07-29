#ifndef CHATEVENTTYPE_H_
#define CHATEVENTTYPE_H_

namespace HovUni {

/**
 * This is an event type for chat events.
 *
 * @author Olivier Berghmans
 */
enum ChatEventType {
	textLine,
	notifyLine
	// Make sure to adjust the amount below
};

/** The amount of event types, used for compressing in the network */
static const unsigned ChatEventTypeSize = 2;

}

#endif
