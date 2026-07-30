#include "GameView.h"
#include <OgreMeshManager.h>
#include <OgreResourceGroupManager.h>
#include "EntityManager.h"

namespace HovUni {

// Define initial global ID
int GameView::mGlobalID = 1;

GameView::GameView(Ogre::SceneManager * sceneMgr) : mSceneMgr(sceneMgr), mID(mGlobalID++) {
	// Create camera for this game view
	mRaceCam = new RaceCamera(mSceneMgr, mID);

	// Light
	// Ogre 14 API fix (docs/porting/ogre-api-gap.md): Ogre::Light::setPosition
	// is a nodeless-positioning-only method (same class of break as
	// RaceCamera.cpp's Camera fix above) and no longer exists against this
	// tree's vcpkg `ogre` port; this light was never attached to a
	// SceneNode at all, so create one to hold its position instead.
	Ogre::Light * light = mSceneMgr->createLight("Light1");
	light->setType(Ogre::Light::LT_POINT);
	Ogre::SceneNode* lightNode = mSceneMgr->getRootSceneNode()->createChildSceneNode("Light1Node", Ogre::Vector3(250, 150, 250));
	lightNode->attachObject(light);
	light->setDiffuseColour(Ogre::ColourValue::White);
	light->setSpecularColour(Ogre::ColourValue::White);
}

GameView::~GameView() {
}

void GameView::addEntityRepresentation(EntityRepresentation * entityRep) {
	// Add to entity representations
	mEntityRepresentations.push_back(entityRep);
}

void GameView::removeEntityRepresentation(Ogre::String entityRep) {
	// Loop through list and remove if found
	for (std::vector<EntityRepresentation *>::const_iterator it = mEntityRepresentations.begin(); 
			it != mEntityRepresentations.end(); it++) {
		if ((*it)->getEntity()->getName() == entityRep) {
			// Erase from entity representations
			mEntityRepresentations.erase(it);
			break;
		}
	}
}

void GameView::draw(Ogre::Real timeSinceLastFrame) {
	// Update the camera
	mRaceCam->update(timeSinceLastFrame);

	// Draw the entity representations
	drawEntityRepresentations(timeSinceLastFrame);
	
	// TODO Draw the static objects
}

void GameView::drawEntityRepresentations(Ogre::Real timeSinceLastFrame) {
	for (std::vector<EntityRepresentation *>::const_iterator it = mEntityRepresentations.begin(); 
			it != mEntityRepresentations.end(); it++) {
		(*it)->draw(timeSinceLastFrame);
	}
}

}