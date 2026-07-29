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
	Ogre::Light * light = mSceneMgr->createLight("Light1");
	light->setType(Ogre::Light::LT_POINT);
	light->setPosition(Ogre::Vector3(250, 150, 250));
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