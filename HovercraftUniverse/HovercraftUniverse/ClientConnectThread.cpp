#include "ClientConnectThread.h"
#include "Ogre.h"

namespace HovUni {
	ClientConnectThread::ClientConnectThread(HUClient* client, ConnectListener* listener) 
			: mClient(client), mListener(listener), mThread(0) {
		
	}

	ClientConnectThread::~ClientConnectThread() {
		delete mThread;
	}

	void ClientConnectThread::start() {
		if (!mThread) {
			mThread = new boost::thread(boost::ref(*this));
		}
	}

	void ClientConnectThread::operator()() {
		Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "[ClientConnectThread]: thread started";

		//Try connecting
		try {
			mClient->connect(0);
			//Process the commands
			mClient->process(1);

			//Try to spam-check for a connection for two seconds
			int retryCount = 0;
			while ( (!mClient->finishedConnecting()) && (retryCount < 20) ) {
				mClient->timed_wait(boost::get_system_time() + boost::posix_time::millisec(100)); //Wait for 100 milliseconds
				mClient->process(100);
				++retryCount;
			}

			if (!mClient->finishedConnecting()) {
				//We might have a delay, try some more
				retryCount = 0;
				while ( (!mClient->finishedConnecting()) && (retryCount < 20) ) {
					mClient->timed_wait(boost::get_system_time() + boost::posix_time::millisec(500)); //Wait for 1/2 second
					mClient->process(500);
					++retryCount;
					Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "[ClientConnectThread]: Connection try " << retryCount;
				}
			}
	
			bool success = mClient->isConnected();

			if (!success) {
				//We are not connected, leave!
				delete mClient;
				Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "[ClientConnectThread]: Connection failed " << retryCount;
			}

			mListener->onConnectFinish(success);
		} catch ( NetworkException ex ){
			//TODO NICK SHOW ERROR BOX
			Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "[ClientConnectThread]: Could not connect to server";
			delete mClient;
			
			mListener->onConnectFinish(false);
		}

		Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "[ClientConnectThread]: thread finished";
	}
}