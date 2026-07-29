#include "HovercraftController.h"
#include "Hovercraft.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <math.h>


using namespace boost::posix_time;


namespace HovUni {

HovercraftController::HovercraftController() : mLast(false, false, false, false, false), mPreviousTime(0) {
}

HovercraftController::~HovercraftController() {

}

std::vector<ControllerEvent*> HovercraftController::getEvents() {
    //Dirk: uncomment this to enable path recording in the log
	//you can remove timestamps from the log with the regex "..:..:..: "
	/*
	ptime epoch(time_from_string("1970-01-01 00:00:00.000"));
	ptime now_2 = microsec_clock::universal_time();
	time_duration duration = now_2 - epoch;
	__int64 milliseconds = duration.total_milliseconds();
	if ((milliseconds - mPreviousTime) > 50) {
		Ogre::LogManager::getSingleton().getDefaultLog()->stream() << getEntity()->getPosition().x << " " << getEntity()->getPosition().y << " " << getEntity()->getPosition().z << " 1";
		mPreviousTime = milliseconds;
	}
	*/

	std::vector<ControllerEvent*> events;
	BasicEntityEvent current(moveForward(), moveBackward(), turnLeft(), turnRight(), reset());

	// Only send an event when there is a change
	if (!(current == mLast) || (mTimer.elapsed() >= 1000)) {
		events.push_back(new BasicEntityEvent(current));
		mLast = current;
		mTimer.restart();
	}
	return events;
}

}
