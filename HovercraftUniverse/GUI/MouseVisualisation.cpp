#include "MouseVisualisation.h"

namespace HovUni {
	MouseVisualisation::MouseVisualisation() : mGuiOverlay(0), mCursorContainer(0) {
		mMaterial = Ogre::MaterialManager::getSingleton().create("MouseCursor/default", "General");
		mCursorContainer = (Ogre::OverlayContainer*) Ogre::OverlayManager::getSingletonPtr()->createOverlayElement("Panel", "MouseCursor");
		mCursorContainer->setMaterialName(mMaterial->getName());
		mCursorContainer->setPosition(0, 0);

		mGuiOverlay = Ogre::OverlayManager::getSingletonPtr()->create("MouseCursor");
		mGuiOverlay->setZOrder(649);
		mGuiOverlay->add2D(mCursorContainer);
		mGuiOverlay->show();
	}

	void MouseVisualisation::setImage(const Ogre::String& filename) {
		mTexture = Ogre::TextureManager::getSingleton().load(filename, "General");

		Ogre::TextureUnitState *pTexState;
		if(mMaterial->getTechnique(0)->getPass(0)->getNumTextureUnitStates())
		{
			pTexState = mMaterial->getTechnique(0)->getPass(0)->getTextureUnitState(0);
		}
		else
		{
			pTexState = mMaterial->getTechnique(0)->getPass(0)->createTextureUnitState( mTexture->getName() );
		}
		pTexState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
		mMaterial->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBT_TRANSPARENT_ALPHA);
	}

	void MouseVisualisation::setWindowDimensions(unsigned int width, unsigned int height) {
		mWindowWidth = (Ogre::Real) ((width > 0) ? width : 1);
		mWindowHeight = (Ogre::Real) ((height > 0) ? height : 1);

		mCursorContainer->setWidth(mTexture->getWidth() / mWindowWidth);
		mCursorContainer->setHeight(mTexture->getHeight() / mWindowHeight);
	}

	void MouseVisualisation::setVisible(bool visible) {
		if(visible) {
			mCursorContainer->show();
		} else {
			mCursorContainer->hide();
		}
	}

	void MouseVisualisation::updatePosition(int x, int y) {
		Ogre::Real rx = x / mWindowWidth;
		Ogre::Real ry = y / mWindowHeight;
		mCursorContainer->setPosition(clamp(rx, 0.0f, 1.0f), clamp(ry, 0.0f, 1.0f));
	}

	Ogre::Real MouseVisualisation::clamp(Ogre::Real a, Ogre::Real min, Ogre::Real max) {
		if (a < min) {
			return min;
		}
		if (a > max) {
			return max;
		} 
		return a;
	}
}