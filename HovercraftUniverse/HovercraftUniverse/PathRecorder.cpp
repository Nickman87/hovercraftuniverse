#include "PathRecorder.h"
#include <boost/ref.hpp>

#include "Hovercraft.h"
#include "boost/date_time/gregorian/gregorian.hpp"
#include <fstream>

namespace HovUni {

	PathRecorder * PathRecorder::mServerThread = 0;

	boost::thread * PathRecorder::mThread = 0;

	PathRecorder::PathRecorder(Hovercraft* hover):
		mHover(hover)
	{
	}

	PathRecorder::~PathRecorder(void){
	}

	void PathRecorder::start( Hovercraft* hover){
		if ( mServerThread == 0 ){
			mServerThread = new PathRecorder(hover);
			mThread = new boost::thread(boost::ref(*mServerThread));
		}
	}

	bool PathRecorder::isRecording(){
		return mServerThread != 0;
	}
	
	void PathRecorder::stop(){
		mServerThread->run = false;
		mThread->join();
		delete mThread;
		mThread = 0;
		delete mServerThread;
		mServerThread = 0;
	}

	void PathRecorder::operator()(){
		run = true;
		
		boost::gregorian::date today = boost::gregorian::day_clock::local_day();
		std::string date = boost::gregorian::to_iso_extended_string(today);
		std::string file = date + mHover->getName() + ".path";
		
		std::fstream str;
		str.open(file.c_str());

		while ( run ){
			Ogre::Vector3 pos = mHover->getPosition();
			str << pos[0] << " " << pos[1] << " " << pos[2] << " 1" << std::endl; 

			// Sleep
			boost::xtime xt;
			boost::xtime_get(&xt, boost::TIME_UTC);
			xt.nsec += 1000000;
			boost::thread::sleep(xt);
		}

		str.flush();
	}

}