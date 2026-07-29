#include "Application.h"
#include "EntityRepresentation.h"
#include <OgreMaxUtilities.hpp>
#include <OgreSubEntity.h>
#include <vector>

namespace HovUni {

EntityRepresentation::EntityRepresentation(Entity * entity, Ogre::String meshFile, Ogre::SceneManager * sceneMgr, Ogre::String resourceGroupName, bool visible, bool castShadows, 
										   Ogre::Real renderingDistance, Ogre::String materialFile, std::vector<Ogre::String> subMaterials, Ogre::SceneNode * node)  
											:  mEntity(entity), mSceneMgr(sceneMgr) {
	// Create entity 
	mOgreEntity = mSceneMgr->createEntity(entity->getName(), meshFile, resourceGroupName);
	OgreMax::OgreMaxUtilities::SetObjectVisibility(mOgreEntity, visible ? OgreMax::Types::OBJECT_VISIBLE : OgreMax::Types::OBJECT_HIDDEN);
	mOgreEntity->setCastShadows(castShadows);
	mOgreEntity->setRenderingDistance(renderingDistance);
	if (!materialFile.empty()) {
		mOgreEntity->setMaterialName(materialFile);
	}
	mTextOverlay = 0;
	//if (false) { //TODO overlays are broken.
	if (mEntity->hasLabel()) {
		Ogre::SceneManager::CameraIterator it = mSceneMgr->getCameraIterator();
		Ogre::Camera* cam = it.getNext(); //Warning! We are only using the first camera, if there are more, need to iterate over them.
		if ((mOgreEntity != 0) && (cam != 0)) {
			mTextOverlay = new ObjectTextDisplay(mOgreEntity, cam->getLodCamera());
			mTextOverlay->enable(true);
			mTextOverlay->setText(entity->getLabel());
		}
	} else {
		Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "[EntityRep] Entity " << mEntity->getName() << "(" << mEntity->getCategory() << ") has no label.";
	}

	// Set subentity materials
	for (unsigned int i = 0; i < subMaterials.size(); i++) {
		Ogre::SubEntity * subentity = mOgreEntity->getSubEntity(i);
		subentity->setMaterialName(subMaterials[i]);
	}

	if (node) {
		// There is a node, so just attach entity, but clear position
		mOrientation = node->getOrientation();
		mOgreNode = node;
		mOgreNode->attachObject(mOgreEntity);
	} else {
		// Create node and attach entity to it
		mOgreNode = mSceneMgr->getRootSceneNode()->createChildSceneNode(entity->getName() + "Node");
		mOgreNode->attachObject(mOgreEntity);
	}
}

EntityRepresentation::~EntityRepresentation() {
	delete mTextOverlay;
	mTextOverlay = 0;
}

void EntityRepresentation::draw(Ogre::Real timeSinceLastFrame) {
	// Update the settings
	//mOgreNode->setPosition(mEntity->getPosition());
	mOgreNode->setPosition(mEntity->getSmoothPosition());
	mOgreNode->setOrientation(mEntity->getQuaternion());
	mOgreNode->rotate(mOrientation);
	//Text overlay
	if (mTextOverlay != 0) {
		mTextOverlay->update();
	}
}

}
