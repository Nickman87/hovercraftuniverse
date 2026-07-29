#include "EntityManager.h" 

namespace HovUni {

EntityManager * EntityManager::mEntityManagerServer = 0;
EntityManager * EntityManager::mEntityManagerClient = 0;
const std::string EntityManager::ALL("ALL");

EntityManager::EntityManager() : mEntityTracked("") {
}

EntityManager::~EntityManager() {
}

void EntityManager::registerEntity(Entity * entity) {
	// Check for existence
	if (getEntity(entity->getName())) {
		// TODO Throw exception, already exists
	}

	// Add to entities and new entities
	mEntities.push_back(entity);
	mNewEntities.push_back(entity);
}

void EntityManager::trackEntity(Ogre::String entityName) {
	mEntityTracked = entityName;
}

void EntityManager::releaseEntity(Ogre::String entityName) {
	// Check for existence
	for (std::vector<Entity *>::const_iterator it = mEntities.begin(); it != mEntities.end(); it++) {
		if ((*it)->getName() == entityName) {
			mEntities.erase(it);
			break;
		}
	}

	// TODO Throw exception, does not exist
}

Entity * EntityManager::getEntity(Ogre::String entityName) {
	// Search entity
	for (std::vector<Entity *>::const_iterator it = mEntities.begin(); it != mEntities.end(); it++) {
		if ((*it)->getName() == entityName) {
			// Return entity
			return (*it);
		}
	}

	// No such entity found, return 0
	return 0;
}

std::vector<Entity *> EntityManager::getEntities(Ogre::String categoryName) {
	std::vector<Entity *> results;

	bool add = (categoryName == ALL);

	// Search entities 
	for (std::vector<Entity *>::const_iterator it = mEntities.begin(); it != mEntities.end(); it++) {
		
		if ( add ){
			results.push_back(*it);
		}
		else if ((*it)->getCategory() == categoryName) {
			// Add entity to list
			results.push_back(*it);
		}
	}

	// Return found list
	return results;
}

std::vector<Entity *> EntityManager::getAllEntities() {
	return mEntities;
}

Entity * EntityManager::getTrackedEntity() {
	// Search and return entity
	//TODO: geeft altijd 0 terug?
	if (mEntityTracked == "") {
		//TODO: Might need an exception?
		return 0;
	}
	
	return getEntity(mEntityTracked);
}

std::vector<Entity *> EntityManager::getNewEntities() { 
	std::vector<Entity *> entitiesToReturn = mNewEntities;
	mNewEntities.clear();

	return entitiesToReturn;
}

std::vector<Entity *> EntityManager::getRemovedEntities() { 
	std::vector<Entity *> entitiesToReturn = mRemovedEntities;
	mRemovedEntities.clear();

	return entitiesToReturn;
}

void EntityManager::updateEntities(Ogre::Real timeSinceLastFrame) {
	// Loop over entities and update them
	for (std::vector<Entity *>::const_iterator it = mEntities.begin(); it != mEntities.end(); it++) {
		(*it)->update(timeSinceLastFrame);
	}
}

EntityManager * EntityManager::getServerSingletonPtr(void) {
	if (!mEntityManagerServer) {
		mEntityManagerServer = new EntityManager();
	}

	return mEntityManagerServer;
}

EntityManager * EntityManager::getClientSingletonPtr(void) {
	if (!mEntityManagerClient) {
		mEntityManagerClient = new EntityManager();
	}

	return mEntityManagerClient;
}

}