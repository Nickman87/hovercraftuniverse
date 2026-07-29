#include "GUIManager.h"

#include "BasicOverlay.h"
#include "OverlayContainer.h"
#include "Ogre.h"

namespace HovUni {
	GUIManager* GUIManager::msSingleton = 0;

	GUIManager::GUIManager(const Ogre::String& mediaPath, Ogre::Viewport* viewport) 
			: mViewport(viewport), mMouseVisual(), mMouse(0), mMouseMoved(false) {
		mHikariMgr = new Hikari::HikariManager(mediaPath.c_str());

		//Fix the overlays going on top
		setBringTotTop(false);

		mMouseVisual.setImage("cursor.png");
		mMouseVisual.setWindowDimensions(getResolutionWidth(), getResolutionHeight());
	}

	GUIManager& GUIManager::getSingleton(void) {
		if (!msSingleton) {
			THROW(UninitializedException, "GUIManager needs to be initialised before you use it.");
		}
		return (*msSingleton);
	}

	GUIManager* GUIManager::getSingletonPtr(void) {
		if (!msSingleton) {
			THROW(UninitializedException, "GUIManager needs to be initialised before you use it.");
		}
		return msSingleton;
	}

	void GUIManager::init(const Ogre::String& mediaPath, Ogre::Viewport* viewport) {
		msSingleton = new GUIManager(mediaPath, viewport);
	}
	
	GUIManager::~GUIManager() {
		if(mHikariMgr) {
			removeAllOverlays();
			delete mHikariMgr;
		}
	}

	void GUIManager::removeOverlay(const Ogre::String& name) {
		mHikariMgr->destroyFlashControl(name);
	}

	void GUIManager::removeAllOverlays() {
		mHikariMgr->destroyAllControls();
	}

	void GUIManager::update() {
		//activate the queued overlays
		while (!mQueuedOverlays.empty()) {
			mQueuedOverlays.back()->activate();
			mQueuedOverlays.pop_back();
		}

		try {
			mHikariMgr->update();
		} catch (...) {
			Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream()
				<< "[GUIManager]: Exception occured ";
		}

		//Execute queued mouse events, inserted after update to prevent crashes
		mMouseMutex.lock();
		if (mMouseMoved) {
			mMouseMoved = false;
			OIS::MouseEvent evt(mMouse, mMouseState);
			mMouseVisual.updatePosition(evt.state.X.abs, evt.state.Y.abs);
			mHikariMgr->injectMouseMove(evt.state.X.abs, evt.state.Y.abs);
			mHikariMgr->injectMouseWheel(evt.state.Z.rel);
		}
		mMouseMutex.unlock();
	}

	bool GUIManager::mouseMoved(const OIS::MouseEvent &evt) {
		mMouseMutex.lock();
		mMouseState = evt.state;
		if (mMouse == 0) {
			mMouse = const_cast<OIS::Object *> (evt.device);
		}
		mMouseMoved = true;
		mMouseMutex.unlock();

		return true;
	}

	bool GUIManager::mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id) {
		mHikariMgr->injectMouseDown(id);

		return true;
	}

	bool GUIManager::mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id) {
		mHikariMgr->injectMouseUp(id);

		return true;
	}

	bool GUIManager::keyPressed(const OIS::KeyEvent &evt) {
		std::vector<OIS::KeyListener*>::iterator it;

		for (it = mKeyListeners.begin(); it != mKeyListeners.end(); ++it) {
			(*it)->keyPressed(evt);
		}

		

		return true;
	}

	bool GUIManager::keyReleased(const OIS::KeyEvent &evt) {
		std::vector<OIS::KeyListener*>::iterator it;

		for (it = mKeyListeners.begin(); it != mKeyListeners.end(); ++it) {
			(*it)->keyReleased(evt);
		}

		return true;
	}

	void GUIManager::addKeyListener(OIS::KeyListener* listener) {
		mKeyListeners.push_back(listener);
	}

	void GUIManager::removeKeyListener(OIS::KeyListener* listener) {
		std::vector<OIS::KeyListener*>::iterator it;

		for (it = mKeyListeners.begin(); it != mKeyListeners.end(); ++it) {
			if ( (*it) == listener ) {
				//Remove it!
				mKeyListeners.erase(it);
				break;
			}
		}
	}

	Hikari::FlashControl* GUIManager::createOverlay(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder) {
		Hikari::FlashControl* flashControl = mHikariMgr->createFlashOverlay(name, mViewport, width, height, position, zOrder);
		flashControl->load(fileName);
		
		return flashControl;
	}

	void GUIManager::activateOverlay(BasicOverlay* overlay) {
		mQueuedOverlays.push_back(overlay);
		//overlay->activate();
	}

	void GUIManager::disableOverlay(BasicOverlay* overlay) {
		overlay->disable();
	}

	void GUIManager::activateOverlayContainer(OverlayContainer* overlayContainer) {
		std::map<std::string, BasicOverlay*> overlays = overlayContainer->getOverlays();

		for(std::map<std::string, BasicOverlay*>::iterator it = overlays.begin(); it != overlays.end(); ++it) {
			(*it).second->activate();
		}
	}
			
	void GUIManager::disableOverlayContainer(OverlayContainer* overlayContainer) {
		std::map<std::string, BasicOverlay*> overlays = overlayContainer->getOverlays();

		for(std::map<std::string, BasicOverlay*>::iterator it = overlays.begin(); it != overlays.end(); ++it) {
			(*it).second->disable();
		}
	}

	int GUIManager::getResolutionWidth() {
		return mViewport->getActualWidth();
	}

	int GUIManager::getResolutionHeight() {
		return mViewport->getActualHeight();
	}

	void GUIManager::showCursor(bool val) {
		mMouseVisual.setVisible(val);
	}

	std::pair<int, int> GUIManager::scale(int minWidth, int minHeight, int maxWidth, int maxHeight, int minResWidth, int minResHeight, int maxResWidth, int maxResHeight) {
		int screenWidth = getResolutionWidth();
		int screenHeight = getResolutionHeight();

		float scale;
		{
			if ( (maxResWidth <= screenWidth) && (maxResHeight <= screenHeight) ) {
				scale = 1;
			} else if ( (minResWidth >= screenWidth) && (minResHeight >= screenHeight) ) {
				scale = 0;
			} else {
				float scaleX = ((float)(screenWidth - minResWidth) / (float)(maxResWidth - minResWidth));
				float scaleY = ((float)(screenHeight - minResHeight ) / (float)(maxResHeight  - minResHeight ));

				scale = (scaleX < scaleY) ? scaleX : scaleY;
			}
		}

		int width = (int) (minWidth + scale * (maxWidth - minWidth));
		int height = (int) (minHeight + scale * (maxHeight - minHeight));

		return std::pair<int, int>(width, height);
	}

	void GUIManager::setBringTotTop(bool val) {
		mHikariMgr->toggleBringToTop(val);
	}
}
