#ifndef TIMING_H_
#define TIMING_H_

#include <boost/date_time/posix_time/posix_time_types.hpp>

namespace HovUni {

/**
 * This is an helper class that provides some timing functions
 *
 * @author Olivier Berghmans
 */
class Timing {
private:
	/** The start time */
	boost::posix_time::ptime mStart;

public:
	/**
	 * Constructor
	 */
	Timing();

	/**
	 * Destructor
	 */
	virtual ~Timing();

	/**
	 * Get the elapsed time
	 *
	 * @return the elapsed time in milliseconds
	 */
	long elapsed() const;

	/**
	 * Restart the timer
	 */
	void restart();
};

}
#endif //TIMING_H_
