#include "BasicEntityEvent.h"
#include <OgreLogManager.h>

namespace HovUni {

BasicEntityEvent::BasicEntityEvent(bool forward, bool backward, bool left, bool right, bool reset) : 
	ControllerEvent(BasicEntity), mForward(forward), mBackward(backward), mLeft(left), mRight(right), mReset(reset) {

}

BasicEntityEvent::BasicEntityEvent() : ControllerEvent(BasicEntity), mForward(false), mBackward(false), mLeft(false), mRight(false), mReset(false) {

}

BasicEntityEvent::~BasicEntityEvent() {

}

bool BasicEntityEvent::operator==(const BasicEntityEvent& event) {
	if ((mForward == event.mForward) && 
		(mBackward == event.mBackward) && 
		(mLeft == event.mLeft) && 
		(mRight == event.mRight)&& 
		(mReset == event.mReset)) {
			return true;
	} else {
		return false;
	}
}

bool BasicEntityEvent::moveForward() const {
	return mForward;
}

bool BasicEntityEvent::moveBackward() const {
	return mBackward;
}

bool BasicEntityEvent::moveLeft() const {
	return mLeft;
}

bool BasicEntityEvent::moveRight() const {
	return mRight;
}

bool BasicEntityEvent::reset() const {
	return mReset;
}

void BasicEntityEvent::clear() {
	mForward = false;
	mBackward = false;
	mLeft = false;
	mReset = false;
	mRight = false;
}

void BasicEntityEvent::write(ZCom_BitStream* stream) const {
	// Write information for controller event
	ControllerEvent::write(stream);

	// Write own information
	stream->addBool(mForward);
	stream->addBool(mBackward);
	stream->addBool(mLeft);
	stream->addBool(mRight);
	stream->addBool(mReset);
}

void BasicEntityEvent::read(ZCom_BitStream* stream) {
	// Read information for controller event
	ControllerEvent::read(stream);

	// Read own information
	mForward = stream->getBool();
	mBackward = stream->getBool();
	mLeft = stream->getBool();
	mRight = stream->getBool();
	mReset = stream->getBool();
}

BasicEntityEvent* BasicEntityEvent::parse(ZCom_BitStream* stream) {
	BasicEntityEvent* e = new BasicEntityEvent();
	e->deserialize(stream);
	return e;
}

}
