#ifndef PATHRECORDER_H
#define PATHRECORDER_H

#include <boost/thread.hpp>

namespace HovUni {

class Hovercraft;

class PathRecorder
{
private:
	Hovercraft* mHover;

	bool run;

	static PathRecorder * mServerThread;

	static boost::thread * mThread;

public:

	PathRecorder(Hovercraft* hover);

	~PathRecorder(void);

	/**
	 * Start recording
	 */
	static void start( Hovercraft* hover);

	static bool isRecording();
	
	/**
	 * Stop recording
	 */
	static void stop();

	void operator()();
};

}

#endif
