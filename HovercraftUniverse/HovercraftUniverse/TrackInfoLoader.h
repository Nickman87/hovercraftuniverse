#ifndef TRACKINFOLOADER_H
#define TRACKINFOLOADER_H

#include "Loader.h"

namespace HovUni {

class Track;

/**
 * Loader for global track info.
 * 
 * @author Kristof Overdulve
 */
class TrackInfoLoader : public Loader {
private:

	/** Filename **/
	Ogre::String mFilename;

	/** The track */
	Track * mTrack;

public:
	/**
	 * Constructor.
	 * @param filename
	 */
	TrackInfoLoader( const Ogre::String& filename );

	/**
	 * Destructor.
	 */ 
	virtual ~TrackInfoLoader();

	/**
	 * Returns the track after the loading or 0 if no loading has occurred yet.
	 *
	 * @return the track
	 */
	Track * getTrack() { return mTrack; }

	/**
	 * @see Loader::onSceneUserData().
	 */
	void onSceneUserData(const Ogre::String& userDataReference, const Ogre::String& userData);

};

}

#endif
