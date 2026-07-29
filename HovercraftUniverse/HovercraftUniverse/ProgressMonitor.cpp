#include "ProgressMonitor.h"
#include <string>

namespace HovUni {

ProgressMonitor* ProgressMonitor::msSingleton = 0;

ProgressMonitor::ProgressMonitor() : mTotalEntitiesToLoad(0), mTotalEntitiesLoaded(0) {
}

ProgressMonitor& ProgressMonitor::getSingleton(void) {
	if (!msSingleton) {
		msSingleton = new ProgressMonitor();
	}

	return *msSingleton;
}

ProgressMonitor* ProgressMonitor::getSingletonPtr(void) {
	if (!msSingleton) {
		msSingleton = new ProgressMonitor();
	}

	return msSingleton;
}

void ProgressMonitor::addTask(std::string title, int noLoads) {
	if (mEntitiesToLoad.size() == 0) {
		// First element --> initialize
		mTotalEntitiesToLoad = 0;
		mTotalEntitiesLoaded = 0;
	}

	mEntitiesToLoad[title] =  noLoads;
	mEntitiesLoaded[title] = 0;
	mTotalEntitiesToLoad += noLoads;
}

void ProgressMonitor::updateTask(std::string title) {
	mEntitiesLoaded[title] ++;
	mTotalEntitiesLoaded ++;

	// Notify listeners
	for (listener_iterator i = listenersBegin(); i != listenersEnd(); ++i) {
		(*i)->updateProgress(getProgress());
	}
}

double ProgressMonitor::getProgress() {
	return (double) mTotalEntitiesLoaded / (double) mTotalEntitiesToLoad;
}

}