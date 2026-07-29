#ifndef PROGRESSMONITOR_H_
#define PROGRESSMONITOR_H_

#include <vector>
#include <utility>
#include <map>
#include "Listenable.h"
#include "ProgressMonitorListener.h"
#include <string>

namespace HovUni {

/**
 * The progress monitor can be used to - here it comes! - monitor progress in a task that takes a while. It allows for the system to give feedback to the
 * user about how long it will take to perform the task(s). TODO Perhaps do something with the entity titles, now they're quite .. useless.
 *
 * @author Kristof Overdulve
 */
	class ProgressMonitor : public Listenable<IProgressMonitorListener>
{
private:
	/** The progressmonitor instance */
	static ProgressMonitor* msSingleton;
	
	/** The entities to load */
	std::map<std::string, int> mEntitiesToLoad;

	/** The entities loaded */
	std::map<std::string, int> mEntitiesLoaded;

	/** The total number of entities to load */
	int mTotalEntitiesToLoad;

	/** The total number of entities loaded */
	int mTotalEntitiesLoaded;

	/** Constructor */
	ProgressMonitor(void);

public:
	/** Destructor */
	virtual ~ProgressMonitor(void) {}

	/**
	 * Get a reference to the ProgressMonitor
	 * @return reference to the ProgressMonitor
	 */
	static ProgressMonitor& getSingleton(void);

	/**
	 * Get a pointer to the ProgressMonitor
	 * @return pointer to the ProgressMonitor
	 */
	static ProgressMonitor* getSingletonPtr(void);

	/**
	 * Includes the given task into the monitor.
	 *
	 * @param title the title of the new task to load
	 * @param noLoads the number of elements that must be loaded
	 */
	void addTask(std::string title, int noLoads);

	/**
	 * Updates the given task in order to indicate that a new element has been loaded.
	 *
	 * @param title the title of the task that is updated
	 */
	void updateTask(std::string title);

	/**
	 * Returns the ratio of number of elements loaded, compared to the total number of entities to load (e.g. return of 0.51 means 51%).
	 *
	 * @return the progress
	 */
	double getProgress();
};

}

#endif