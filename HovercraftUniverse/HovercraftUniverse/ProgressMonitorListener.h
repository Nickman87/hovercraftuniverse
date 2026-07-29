#ifndef PROGRESSMONITORLISTENER_H_
#define PROGRESSMONITORLISTENER_H_

namespace HovUni {

/**
 * The progress monitor listener that must be implemented in order to be automatically updated with new progress change events.
 *
 * @author Kristof Overdulve
 */
class IProgressMonitorListener {
	
public:

	/**
	 * Must be overriden to specify behaviour to occur when the progress has changed.
	 *
	 * @param progress the current progress after the update
	 */
	virtual void updateProgress(double progress) = 0;

};

}


#endif