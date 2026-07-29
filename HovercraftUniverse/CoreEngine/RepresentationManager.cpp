#include "RepresentationManager.h"
#include <cassert>
#include <GUIManager.h>
#include <SoundManager.h>

namespace HovUni {

RepresentationManager * RepresentationManager::mRepresentationManager = 0;

RepresentationManager::RepresentationManager(EntityManager * entityMgr, Ogre::SceneManager * sceneMgr) 
		: mEntityManager(entityMgr), mSceneMgr(sceneMgr) {
}

void RepresentationManager::initialise(EntityManager * entityMgr, Ogre::SceneManager * sceneMgr) {
	mRepresentationManager = new RepresentationManager(entityMgr, sceneMgr);
}

RepresentationManager::~RepresentationManager() {
}

void RepresentationManager::addEntityRepresentation(EntityRepresentation * entityRep) {
	// Check for existence
	if (mEntityManager->getEntity(entityRep->getEntity()->getName())) {
		// TODO Throw exception, already exists
	}

	// Add to entity representations
	mEntityRepresentations.push_back(entityRep);

	// Add to all game views
	for (std::vector<GameView *>::const_iterator it = mGameViews.begin(); it != mGameViews.end(); it++) {
		(*it)->addEntityRepresentation(entityRep);
	}
}

void RepresentationManager::removeEntityRepresentation(Ogre::String entityRep) {
	// Loop through list and remove if found
	for (std::vector<EntityRepresentation *>::const_iterator it = mEntityRepresentations.begin(); it != mEntityRepresentations.end(); it++) {
		if ((*it)->getEntity()->getName() == entityRep) {
			// Erase from entity representations
			mEntityRepresentations.erase(it);

			// Erase from game views
			for (std::vector<GameView *>::const_iterator it = mGameViews.begin(); it != mGameViews.end(); it++) {
				(*it)->removeEntityRepresentation(entityRep);
			}
			break;
		}
	}
}

void RepresentationManager::addGameView(GameView * gameView) {
	// Add the game view to the list of game views
	mGameViews.push_back(gameView);
}

RepresentationManager * RepresentationManager::getSingletonPtr(void) {
	// Representation manager must have been initialized
	assert (mRepresentationManager);

	// Return
	return mRepresentationManager;
}

void RepresentationManager::updateRepresentations() {
	// Update the representation
	for (std::vector<EntityRepresentation *>::const_iterator it = mEntityRepresentations.begin(); 
			it != mEntityRepresentations.end(); it++) {
		// Update representation
		// TODO On the diagram it says update, but we don't do updates, we just draw
	}
}

void RepresentationManager::drawGameViews(Ogre::Real timeSinceLastFrame) {
	// Draw all game views
	for (std::vector<GameView *>::const_iterator it = mGameViews.begin(); it != mGameViews.end(); it++) {
		(*it)->draw(timeSinceLastFrame);
	}
}

EntityRepresentation* RepresentationManager::getTrackedEntityRepresentation() {
	Entity* trackedEntity = mEntityManager->getTrackedEntity();

	if (trackedEntity == 0) {
		return 0;
	}

	for (std::vector<EntityRepresentation *>::const_iterator it = mEntityRepresentations.begin(); 
			it != mEntityRepresentations.end(); it++) {
		
		if ((*it)->getEntity() == trackedEntity) {
			return (*it);
		}
	}

	return 0;
}


}