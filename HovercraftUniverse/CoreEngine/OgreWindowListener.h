#include <OgreRoot.h>
#include <OgreWindowEventUtilities.h>

namespace HovUni {
	class OgreWindowListener: public Ogre::WindowEventListener {
	public:
		OgreWindowListener();
		bool windowClosing(Ogre::RenderWindow* rw);
	};
}