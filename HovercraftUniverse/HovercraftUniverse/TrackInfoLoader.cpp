#include "CustomOgreMaxScene.h"
#include "TrackInfoLoader.h"
#include "Track.h"
#include <OgreLogManager.h>

namespace HovUni {

TrackInfoLoader::TrackInfoLoader( const Ogre::String& filename) : mTrack(0), mFilename(filename) {
	load(filename);
}

TrackInfoLoader::~TrackInfoLoader(void) {
	if ( mTrack != 0 ){
		delete mTrack;
	}
}

void TrackInfoLoader::onSceneUserData(const Ogre::String& userDataReference, const Ogre::String& userData) {
	Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "TRACKINFOLOADER USERDATA";
	if (!userData.empty()) {
		TiXmlDocument mDocument;
		mDocument.Parse(userData.c_str());
		TiXmlElement * root = mDocument.RootElement();

		if(root){
		
			if ( mTrack ){
				delete mTrack;
				mTrack = 0;
			}

			mTrack = new Track();
			mTrack->load(root);
		}

		//clear xml
		mDocument.Clear();		
	}
}

}
