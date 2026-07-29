#include "OverlayContainer.h"

#include "GUIManager.h"

namespace HovUni {
	OverlayContainer::OverlayContainer() : mIsActivated(false) {

	}

	void OverlayContainer::addOverlay(const std::string& name, BasicOverlay * overlay) {
		//Check for duplicates
		if (mOverlays.find(name) == mOverlays.end()) {
			mOverlays[name] = overlay;

			if (mIsActivated) {
				//Activate the newly added overlay too
				GUIManager::getSingletonPtr()->activateOverlay(overlay);
			}
		}
	}

	void OverlayContainer::removeOverlay(const std::string& name) {
		// Check if item exists
		if (mOverlays.find(name) != mOverlays.end()) {
			if (mIsActivated) {
				//Disable the overlay first
				GUIManager::getSingletonPtr()->disableOverlay(mOverlays[name]);
			}

			mOverlays.erase(mOverlays.find(name));
		} else {
			// Doesn't Exist
		}
	}

	void OverlayContainer::activate() {
		mIsActivated = true;
		GUIManager::getSingletonPtr()->activateOverlayContainer(this);
		onActivate();
	}

	void OverlayContainer::deactivate() {
		mIsActivated = false;
		onDeactivate();
		GUIManager::getSingletonPtr()->disableOverlayContainer(this);
	}

	std::map<std::string, BasicOverlay *>& OverlayContainer::getOverlays() {
		return mOverlays;
	}

}