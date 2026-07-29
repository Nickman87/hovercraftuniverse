#ifndef OVERLAYCONTAINER_H
#define OVERLAYCONTAINER_H

#include "BasicOverlay.h"
#include <string>

namespace HovUni{
	/**
	 * This class is container for BasicOverlay objects. This class will make it
	 * easier to links multiple overlays to activate and deactivate at the same time.
	 *
	 * @author Nick De Frangh
	 */
	class OverlayContainer {
		friend class GUIManager;

		private:
			/** A map of all the contained overlays */
			std::map<std::string, BasicOverlay *> mOverlays;

			/** A boolean to check if this container is actived or not */
			bool mIsActivated;

		public:
			/** Besic constructor */
			OverlayContainer();

			/**
			 * Add an overlay to this container
			 *
			 * @param name The name for the overlay you are adding
			 * @param overlay The overlay you want to add.
			 */
			void addOverlay(const std::string& name, BasicOverlay * overlay);

			/** 
			 * Remove an overlay out of the container
			 *
			 * @param name The name of the overlay
			 */
			void removeOverlay(const std::string& name);

			/**
			 * Function called when this container is activated.
			 */
			virtual void onActivate() {}

			/**
			 * Function called when this container is deactivated.
			 */
			virtual void onDeactivate() {}

			/**
			 * Draw the overlay container
			 */
			void activate();

			/**
			 * Hide the overlay container
			 */
			void deactivate();

			/** 
			 * Indicates whether the overlay container is visible or hidden.
			 *
			 * @return true if the overlay container is visible and false if hidden
			 */
			inline bool isActivated() { return mIsActivated; }

		private:
			/**
			 * Get a map of all the overlays in this container
			 *
			 * @return A full map over overlays.
			 */
			std::map<std::string, BasicOverlay *>& getOverlays();
	};
}

#endif //OVERLAYCONTAINER_H