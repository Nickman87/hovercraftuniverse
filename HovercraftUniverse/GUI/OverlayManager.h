#ifndef _OVERLAYMANAGER_H
#define _OVERLAYMANAGER_H

#include "BasicOverlay.h"
#include <map>
#include "OverlayParameters.h"

namespace HovUni {
	class OverlayManager {
		private:
			std::map<Ogre::String, BasicOverlay*> mOverlayMapping;

			//std::map<Ogre::String, 

		public:
			OverlayManager();
			virtual ~OverlayManager();

			void addOverlay(BasicOverlay* newOverlay);

			//void
	};
}

#endif //_OVERLAYMANAGER_H
