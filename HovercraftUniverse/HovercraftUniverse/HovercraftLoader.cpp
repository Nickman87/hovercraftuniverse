#include "Hovercraft.h"
#include "HovercraftRepresentation.h"
#include "HovercraftLoader.h"
#include "GameView.h"
#include "RepresentationManager.h"
#include "ProgressMonitor.h"
#include <OgreSceneManager.h>

namespace HovUni {


HovercraftLoader::HovercraftLoader(Ogre::SceneManager * sceneMgr, Ogre::String hovName) : mSceneMgr(sceneMgr), mCurrNode(0), mHovName(hovName) {
	// Empty
}

HovercraftLoader::~HovercraftLoader(void) {
	// Empty
}
	
void HovercraftLoader::StartedLoad() {
	// Empty
}


void HovercraftLoader::FinishedLoad( bool success ) {
	if (success && mCurrEntity && mCurrNode) {		
		// Update the loading of the hovercrafts
		ProgressMonitor::getSingletonPtr()->updateTask("Loading hovercraft.");
	}
}

void HovercraftLoader::onEntity(OgreMax::Types::EntityParameters& entityparameters, const OgreMax::Types::Attachable * parent) {
	Ogre::Log::Stream stream = Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream();
	stream  << "onEntity";
	Ogre::String userData = entityparameters.extraData->userData;

	if (!userData.empty()) {
		// Fetch the type of entity
		TiXmlDocument mDocument;
		mDocument.Parse(userData.c_str());
		TiXmlElement * root = mDocument.RootElement();

		// If correct xml, read it
		if (root) {
			// Read and set the entity type
			TiXmlElement * gameEntityElm = dynamic_cast<TiXmlElement *>(root->FirstChild());
			Ogre::String entityName = Ogre::String(gameEntityElm->GetText());

			// Read the sub materials
			std::vector<Ogre::String> subMaterials;
			for (size_t subentityIndex = 0; subentityIndex < entityparameters.subentities.size(); subentityIndex++) {
				if (!entityparameters.subentities[subentityIndex].materialName.empty()) {
					subMaterials.push_back(entityparameters.subentities[subentityIndex].materialName);
				}
			}

			// Get the entity to which the representation should refer
			Entity * ent = EntityManager::getClientSingletonPtr()->getEntity(mHovName);
			if (ent) {
				// Create hovercraft representation
				EntityRepresentation * entRep = new HovercraftRepresentation(dynamic_cast<Hovercraft *>(ent), mSceneMgr, entityparameters.meshFile, entityparameters.resourceGroupName, true, entityparameters.castShadows, 
					entityparameters.renderingDistance, entityparameters.materialFile, subMaterials, mCurrNode);
				if (entRep) {
					// Add to representation manager
					Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "HovercraftLoader, success";
					RepresentationManager::getSingletonPtr()->addEntityRepresentation(entRep);
				}
				mCurrEntity = entRep;
			} else {
				// The entity did not exist
				Ogre::Log::Stream stream = Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream();
				stream << "Hovercraft with name |" << mHovName << "| does not exist in client." << "\n";

				stream << "Entities are: " << "\n";
				std::vector<Entity *> entities = EntityManager::getClientSingletonPtr()->getAllEntities();
				for (std::vector<Entity *>::const_iterator it = entities.begin(); it != entities.end(); it++) {
					stream << (*it)->getName() << "\t\t" << (*it)->getCategory() << "\n";
				}
			}

			// Clear xml
			mDocument.Clear();
		}
	}
}

void HovercraftLoader::onNode(OgreMax::Types::NodeParameters& nodeparameters, std::vector<OgreMax::Types::NodeAnimation> * animation, const OgreMax::Types::NodeParameters* parent) {
	Ogre::Log::Stream stream = Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream();
	stream  << "onNode";
	Ogre::String name = nodeparameters.name;
	

	// Create node
	Ogre::SceneNode * node = mSceneMgr->getRootSceneNode()->createChildSceneNode();
	node->setOrientation(nodeparameters.orientation);
	node->scale(nodeparameters.scale);

	// Build the animation if there is one
	if (animation) {
		for (std::vector<OgreMax::Types::NodeAnimation>::const_iterator it = animation->begin(); it != animation->end(); it++) {
			Ogre::Animation * animation;
			if (mSceneMgr->hasAnimation(it->parameters.name)) {
				// Just fetch animation
				animation = mSceneMgr->getAnimation(it->parameters.name);
			} else {
				// Create new animation
				animation = mSceneMgr->createAnimation(it->parameters.name, it->parameters.length);
				animation->setInterpolationMode(it->parameters.interpolationMode);
				animation->setRotationInterpolationMode(it->parameters.rotationInterpolationMode);
			}

			// Creating animation track for the node
			Ogre::NodeAnimationTrack * animationTrack = animation->createNodeTrack(animation->getNumNodeTracks() + 1, node);

			// Create key frames
			for (std::vector<OgreMax::Types::KeyFrame>::const_iterator itFrame = it->animationTrack.begin(); itFrame != it->animationTrack.end(); itFrame ++) {
				Ogre::TransformKeyFrame * keyFrame = animationTrack->createNodeKeyFrame(itFrame->keyTime);
				keyFrame->setTranslate(Ogre::Vector3(0.0, 0.0, 0.0));//  itFrame->translation);
				keyFrame->setRotation(itFrame->rotation);
				keyFrame->setScale(itFrame->scale);
			}

			// Now load the animation into runtime
			// Create the animation state, since it doesn't exist yet
			if (!mSceneMgr->hasAnimationState(it->parameters.name)) {
				Ogre::AnimationState * animationState = mSceneMgr->createAnimationState(it->parameters.name);
				animationState->setEnabled(/*it->parameters.enable*/false);
				animationState->setLoop(it->parameters.looping);
			}

			mSceneMgr->getAnimationState(it->parameters.name);
		}
	}

	// Store node as current node
	mCurrNode = node;
}

}