#include "Timing.h"

namespace HovUni {

Timing::Timing() : mStart(boost::posix_time::microsec_clock::universal_time()) {

}

Timing::~Timing() {

}

long Timing::elapsed() const {
	boost::posix_time::ptime t(boost::posix_time::microsec_clock::universal_time());
	boost::posix_time::time_duration since = t - mStart;
	return (long) since.total_milliseconds();
}

void Timing::restart() {
	mStart = boost::posix_time::microsec_clock::universal_time();
}

}
