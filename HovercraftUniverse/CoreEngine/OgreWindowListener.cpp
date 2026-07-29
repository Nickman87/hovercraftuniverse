#include "OgreWindowListener.h"

namespace HovUni {
	OgreWindowListener::OgreWindowListener() {}

	bool OgreWindowListener::windowClosing(Ogre::RenderWindow *rw) {
        // Disables standard exit methods such as ALT-F4 and the close button
        return false;
    }
};