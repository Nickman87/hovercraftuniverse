#include "HUServerThread.h"
#include "Havok.h"
#include "EntityManager.h"
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/xtime.hpp>
#include <OgreLogManager.h>

namespace HovUni {

HUServerThread::HUServerThread() : mServerCore(0), mStop(false) {
	mServerCore = new HUServerCore();
	Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "[Server]: Ready for incoming connections";
}

HUServerThread::~HUServerThread() {
	delete mServerCore;
	mServerCore = 0;
}

void HUServerThread::stop() {
	mStop = true;
}

void HUServerThread::operator()() {
	boost::posix_time::ptime lastTime(boost::posix_time::microsec_clock::universal_time());

	while (!mStop) {
		// Get elapsed time
		boost::posix_time::ptime t(boost::posix_time::microsec_clock::universal_time());
		boost::posix_time::time_duration since = t - lastTime;
		lastTime = t;

		// Update entity and process server
		//Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "Updating entities " << (float) since.total_microseconds() / (float) 1000000;
		EntityManager::getServerSingletonPtr()->updateEntities((float) since.total_microseconds() / (float) 1000000);
		//Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "Processing server core";
		mServerCore->process((int) since.total_milliseconds());

		// Sleep
		boost::xtime xt;
        boost::xtime_get(&xt, boost::TIME_UTC);
        xt.nsec += 1000000;
        boost::thread::sleep(xt);
	}


}


}
