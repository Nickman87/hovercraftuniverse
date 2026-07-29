#ifndef GUIMANAGER_H
#define GUIMANAGER_H

//Include IOS but prevent double link errors!
#define OIS_DYNAMIC_LIB
#include <OIS.h>

#include <Hikari.h>
#include <Ogre.h>
#include <vector>
#include <utility>

#include <Exception.h>
#include "MouseVisualisation.h"
#include <boost/signals2/mutex.hpp>

namespace HovUni {
	class BasicOverlay; //Forward declaration
	class OverlayContainer;

	/**
	 * The GUIManager class will manage everything that is related to
	 * the graphical user interface that users will see during the game.
	 *
	 * @author Nick De Frangh
	 */
	class GUIManager : public OIS::KeyListener, public OIS::MouseListener {
		private:
			/** The only instance of the GUIManager available */
			static GUIManager * msSingleton;

			/** The viewport for the GUI */
			Ogre::Viewport* mViewport;

			/** The main manager for all the flash objects */
			Hikari::HikariManager* mHikariMgr;

			/** A list of items who asked for keyboard events */
			std::vector<OIS::KeyListener*> mKeyListeners;

			/** A visualisation of the mouse */
			MouseVisualisation mMouseVisual;

			/** Overlays that are queued for activation */
			std::vector<BasicOverlay*> mQueuedOverlays;
			
			/** The resulting mouse state to trigger */
			OIS::MouseState mMouseState;

			/** A pointer to the mouse object */
			OIS::Object *mMouse;

			/** Boolean to check if the mouse has moved */
			bool mMouseMoved;

			/** Mutex for mouse movements */
			boost::signals2::mutex mMouseMutex;

		protected:
			/**
			 * Basic constructor for the GUI manager.
			 *
			 * @param mediaPath The path to where the flash files for the GUI are
			 *		located. This can be absolute or relative
			 * @param viewport The viewport to show the overlays on
			 */
			GUIManager(const Ogre::String& mediaPath, Ogre::Viewport* viewport);

		public:
			/**
			 * Get a reference to the GUIManager
			 * @return reference to the GUIManager
			 * @throws UninitialisedException This means you have not initialised this singleton
			 *		using the init function.
			 */
			static GUIManager& getSingleton(void);

			/**
			 * Get a pointer to the GUIManager
			 * @return pointer to the GUIManager
			 * @throws UninitialisedException This means you have not initialised this singleton
			 *		using the init function.
			 */
			static GUIManager* getSingletonPtr(void);

			/**
			 * Initialisation function, should be the first one to call so that
			 * the instance get initialised. If you dont do this, you will get
			 * an UninitializedException when trying to request the singelton.
			 *
			 * @param mediaPath The path to where the flash files for the GUI are
			 *		located. This can be absolute or relative
			 * @param viewport The viewport to show the overlays on
			 */
			static void init(const Ogre::String& mediaPath, Ogre::Viewport* viewport);

			/**
			 * Destructor
			 */
			virtual ~GUIManager();
		
			/**
			 * Remove one overlay from the current GUI, this will remove the object completely from memory.
			 *
			 * @param name The name of this overlay (the same name you used when you created it)
			 */
			void removeOverlay(const Ogre::String& name);
			
			/**
			 * Remove all current overlays. Could be used to switch from interface completely.
			 */
			void removeAllOverlays();
		
			/**
			 * Update function has to be called everytime the update loop of the
			 * program is executed to perform visual changes.
			 */
			void update();
		
			/**
			 * Inject mouse movement events into the GUI.
			 *
			 * @param evt The MouseEvent that occured
			 * @return ?
			 */
			bool mouseMoved(const OIS::MouseEvent &evt);
		
			/**
			 * Inject mouse press events into the GUI.
			 *
			 * @param evt The MouseEvent that occured
			 * @param id The id of the mouse button that was pressed
			 * @return ?
			 */
			bool mousePressed(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
		
			/**
			 * Inject mouse release events into the GUI.
			 *
			 * @param evt The MouseEvent that occured
			 * @param id The id of the mouse button that was released
			 * @return ?
			 */
			bool mouseReleased(const OIS::MouseEvent &evt, OIS::MouseButtonID id);
			
			/**
			 * Inject key press events into the GUI.
			 *
			 * @param evt The KeyEvent that occured
			 * @return ?
			 */
			bool keyPressed(const OIS::KeyEvent &evt);
		
			/**
			 * Inject key release events into the GUI.
			 *
			 * @param evt The KeyEvent that occured
			 * @return ?
			 */
			bool keyReleased(const OIS::KeyEvent &evt);

			/**
			 * Add a keylistener to the GUIManager. This will most likely be a
			 * @link{BasicOverlay} object.
			 *
			 * @param listener The listener you want to add.
			 */
			void addKeyListener(OIS::KeyListener* listener);

			/**
			 * Remove a keylistener from the GUIManager.
			 *
			 * @param listener The listener to remove.
			 */
			void removeKeyListener(OIS::KeyListener* listener);

			/**
			 * Call this function to activate a new flash file as overlay.
			 *
			 * @param name The name you want to give this overlay
			 * @param fileName The filename of the flash file you want to use
			 * @param viewport The ogre viewport, needed to be able to render the 
			 * @param width The width of the overlay (in pixels)
			 * @param height The height of the overlay (in pixels)
			 * @param position The position where you want to place the overlay, relative to the viewport
			 * @param zOrder The z-order of the overlay. Specify '0' to automatically use the next-highest z-order (default).
			 */
			Hikari::FlashControl* createOverlay(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder = 0);

			/**
			 * Activate an earlier created overlay.
			 *
			 * @param overlay The overlay to activate
			 */
			void activateOverlay(BasicOverlay* overlay);
			
			/**
			 * Disable an earlier created overlay.
			 *
			 * @param overlay The overlay to deactivate
			 */
			void disableOverlay(BasicOverlay* overlay);

			/**
			 * Activate an earlier created overlay container.
			 *
			 * @param overlayContainer The overlay container to activate
			 */
			void activateOverlayContainer(OverlayContainer* overlayContainer);
			
			/**
			 * Disable an earlier created overlay container.
			 *
			 * @param overlayContainer The overlay container to deactivate
			 */
			void disableOverlayContainer(OverlayContainer* overlayContainer);

			/**
			 * Get the width of the viewport
			 *
			 * @return The width of the viewport
			 */
			int getResolutionWidth();

			/**
			 * Get the height of the viewport
			 *
			 * @return The width of the viewport
			 */
			int getResolutionHeight();

			/**
			 * Call this function to show/hide the mouse cursor
			 *
			 * @param val If you pass TRUE the mouse will become visible,
			 *		false will hide the cursor.
			 */
			void showCursor(bool val);

			/**
			 * Scale a given width and height to the current resolution
			 */
			std::pair<int, int> scale(int minWidth, int minHeight, int maxWidth, int maxHeight, int minResWidth = 800, int minResHeight = 600, int maxResWidth = 1680, int maxResHeight = 1050);

			/**
			 * Set if we want to bring elements to the top when focussed.
			 *
			 * @param val True when you want to bring overlays to the top, false otherwise.
			 */
			void setBringTotTop(bool val);
	};
}

#endif //GUIMANAGER_H