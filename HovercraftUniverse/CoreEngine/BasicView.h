#ifndef _BASICVIEW_H
#define _BASICVIEW_H

#include <OgreRoot.h>
#include <OgreCamera.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>

namespace HovUni {

/**
 * A basic view which contains a viewport and a camera. This view is meant to be
 * expanded with more functionality.
 *
 * @author Nick De Frangh
 */
class BasicView {
	protected:
		/** The Ogre render window */
		Ogre::RenderWindow* mRenderWindow;

		/** The Ogre scene manager */
		Ogre::SceneManager* mSceneManager;

		/** The name of this view */
		Ogre::String mViewName;

		/** The Ogre camera of the view */
		Ogre::Camera * mOgreCamera;

		/** A viewport associated with this view */
		Ogre::Viewport* mViewport;

		/** Boolean to check if this view is activated or not */
		bool mActivated;

		/** Position of the near clipping pane */
		float mNearClipDistance;

		/** Position of he far clipping pane */
		float mFarClipDistance;

		/** The relative order of the viewport with others on the target. */
		int mZOrder;

		/** The relative position of the left of the viewport on the target, as a value between 0 and 1. */
		float mLeft;

		/** The relative position of the top of the viewport on the target, as a value between 0 and 1. */
		float mTop;

		/** The relative width of the viewport on the target, as a value between 0 and 1. */
		float mWidth;

		/** The relative height of the viewport on the target, as a value between 0 and 1. */
		float mHeight;

	public:

		/**
		 * Constructor
		 * @param renderWindow		The render window this view uses
		 * @param sceneManager		The sceneManager this view will use
		 * @param name				The name of this view
		 * @param nearClipDistance	Position of the near clipping pane
		 * @param farClipDistance	Position of the far clipping pane
		 * @param ZOrder 			The relative order of the viewport with others on the target (allows overlapping viewports i.e. picture-in-picture). Higher ZOrders are on top of lower ones. The actual number is irrelevant, only the relative ZOrder matters (you can leave gaps in the numbering)
		 * @param left 				The relative position of the left of the viewport on the target, as a value between 0 and 1.
		 * @param top 				The relative position of the top of the viewport on the target, as a value between 0 and 1.
		 * @param width				The relative width of the viewport on the target, as a value between 0 and 1.
		 * @param height 			The relative height of the viewport on the target, as a value between 0 and 1. 
		 */
		BasicView(Ogre::RenderWindow* renderWindow, Ogre::SceneManager* sceneManager, const Ogre::String& name, float nearClipDistance = 0.1f, float farClipDistance = 30000.0f, int zOrder = 0, float left = 0.0f, float top = 0.0f, float width = 1.0f, float height = 1.0f);

		/**
		 * Destructor.
		 */
		virtual ~BasicView();

	private:
		/**
		 * General initialisation function
		 */
		void init();

		/**
		 * Activate this view
		 */
		void activate();

		/**
		 * Deactivate this view
		 */
		void deactivate();

	public:
		/**
		 * Optional pre activation function to be implemented by sub-classes
		 */
		virtual void preActivate() {};

		/**
		 * Optional post activation function to be implemented by sub-classes
		 */
		virtual void postActivate() {};

		/**
		 * Optional pre deactivation function to be implemented by sub-classes
		 */
		virtual void preDeactivate() {};

		/**
		 * Optional post deactivation function to be implemented by sub-classes
		 */
		virtual void postDeactivate() {};

		/**
		 * Returns the scene manager associated with this view.
		 *
		 * @return the scenemanager
		 */
		Ogre::SceneManager* getSceneManager() { return mSceneManager; }

		/**
		 * Returns the name of this view.
		 *
		 * @return the name
		 */
		Ogre::String getViewName() { return mViewName; }

		/**
		 * Returns the camera associated with this view.
		 *
		 * @return the camera
		 */
		Ogre::Camera* getCamera() { return mOgreCamera; }

		/**
		 * Returns the viewport associated with this view.
		 *
		 * @return the viewport
		 */
		Ogre::Viewport* getViewport() { return mViewport; }

		/**
		 * Check if this view is activated.
		 *
		 * @return True if the view is activated, false otherwise
		 */
		bool isActivated() { return mActivated; }
	};
}

#endif //_BASICVIEW_H