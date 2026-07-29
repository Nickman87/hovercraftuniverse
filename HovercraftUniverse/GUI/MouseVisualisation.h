#ifndef MOUSEVISUALISATION_H
#define MOUSEVISUALISATION_H

#include <Ogre.h>
#include <OgrePanelOverlayElement.h>

namespace HovUni {
	/**
	 * This class is used to visualise a mouse cursor in Ogre.
	 *
	 * @author Nick De Frangh & Internet
	 */
	class MouseVisualisation {
		private:
			/** The overlay used to visualize the cursor */
			Ogre::Overlay* mGuiOverlay;

			/** A container to put the mouse visualisation in */
			Ogre::OverlayContainer* mCursorContainer;

			/** The texture the mouse cursor will have */
			Ogre::TexturePtr mTexture;

			/** The material the mouse is made of */
			Ogre::MaterialPtr mMaterial;

			/** The width of the window */
			Ogre::Real mWindowWidth;

			/** The height of the window */
			Ogre::Real mWindowHeight;

		public:
			/**
			 * Constructor
			 */
			MouseVisualisation();

			/**
			 * Destructor
			 */
			~MouseVisualisation(void) {}

			/**
			 * Give the mouse a certain image to visualise
			 *
			 * @param filename The name of the file you want to use.
			 *		Thise file should reside in the Ogre data path
			 */
			void setImage(const Ogre::String& filename);

			/**
			 * Set the window dimensions, should be called whenever the window
			 * changes size.
			 *
			 * @param width The width of the screen
			 * @param height The height of the screen
			 */
			void setWindowDimensions(unsigned int width, unsigned int height);

			/**
			 * Make the cursor visible or invisible
			 *
			 * @param visible True when you want to see the cursor, false otherwise
			 */
			void setVisible(bool visible);

			/**
			 * Update the mouse position. Call this function every frame.
			 *
			 * @param x The new x coordinate
			 * @param y The new y coordinate
			 */
			void updatePosition(int x, int y);

			/**
			 * Simple function to prevent a from going above max or below min.
			 *
			 * @param a The value to check
			 * @param min The minimum value for a.
			 * @param max The maximum value for a.
			 */
			Ogre::Real clamp(Ogre::Real a, Ogre::Real min, Ogre::Real max);
	};
}
#endif //MOUSEVISUALISATION_H
