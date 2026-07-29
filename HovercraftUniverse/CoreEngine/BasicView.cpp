#include "BasicView.h"

namespace HovUni {
	BasicView::BasicView(Ogre::RenderWindow* renderWindow, Ogre::SceneManager* sceneManager, const Ogre::String& name, float nearClipDistance, float farClipDistance, int zOrder, float left, float top, float width, float height) 
		: mRenderWindow(renderWindow), mSceneManager(sceneManager), mViewName(name), mNearClipDistance(nearClipDistance), mFarClipDistance(farClipDistance), mZOrder(zOrder), mLeft(left), mTop(top), mWidth(width), mHeight(height) {
		init();
	}

	BasicView::~BasicView() {
	}

	void BasicView::init() {
		//Make sure everything is initialised
		mOgreCamera = 0;
		mViewport = 0;
		mActivated = false;
	}

	void BasicView::activate() {
		if (!isActivated()) {
			preActivate();

			//Create a camera for this view
			//TODO: We might need some sort of naming convention?
			mOgreCamera = mSceneManager->createCamera(mViewName+"_CAM");
			mOgreCamera->setNearClipDistance(mNearClipDistance);
			mOgreCamera->setFarClipDistance(mFarClipDistance);

			//Create a viewport for this view
			mViewport = mRenderWindow->addViewport(mOgreCamera, mZOrder, mLeft, mTop, mWidth, mHeight);

			//Fix the aspect ratio
			mOgreCamera->setAspectRatio(Ogre::Real(mViewport->getActualWidth()) / Ogre::Real(mViewport->getActualHeight()));

			mActivated = true;

			postActivate();
		}
	}

	void BasicView::deactivate() {
		if (isActivated()) {
			preDeactivate();

			//Remove the viewport
			mRenderWindow->removeViewport(mViewport->getZOrder());
			//delete mViewport;
			mViewport = 0;

			//Delete the camera
			mSceneManager->destroyCamera(mOgreCamera);
			//delete mOgreCamera;
			mOgreCamera = 0;
			
			mActivated = false;

			postDeactivate();
		}
	}
}