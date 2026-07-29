#include "HUServer.h"
#include <boost/ref.hpp>

namespace HovUni {

HUServer::HUServer() : mThread(0), mServerThread(0) {

}


HUServer::~HUServer() {
}

void HUServer::start() {
	if (!mThread && !mServerThread) {
		mServerThread = new HUServerThread();
		mThread = new boost::thread(boost::ref(*mServerThread));
	}
}

void HUServer::stop() {
	if (mThread && mServerThread) {
		mServerThread->stop();
		mThread->join();
		delete mThread;
		mThread = 0;
		delete mServerThread;
		mServerThread = 0;
	}
}

bool HUServer::isRunning() {
	return (mThread != 0);
}

void HUServer::join() {
	if (mThread) {
		mThread->join();
	}
}

}