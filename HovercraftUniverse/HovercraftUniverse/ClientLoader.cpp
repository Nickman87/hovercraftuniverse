#include "GameEntities.h"

#include "AsteroidRepresentation.h"
#include "BoostRepresentation.h"
#include "CheckPointRepresentation.h"
#include "FinishRepresentation.h"
#include "HovercraftRepresentation.h"
#include "PortalRepresentation.h"
#include "StartRepresentation.h"
#include "StaticBodyRepresentation.h"

#include "ClientLoader.h"
#include "GameView.h"
#include "ProgressMonitor.h"
#include "RepresentationManager.h"
#include <OgreSceneManager.h>

namespace HovUni {


ClientLoader::ClientLoader(Ogre::SceneManager * sceneMgr) : mSceneMgr(sceneMgr), mCurrNode(0) {
	// Empty
}

ClientLoader::~ClientLoader(void) {
	// Empty
}

void ClientLoader::StartedLoad(){
	//TODO notify server??
}

void ClientLoader::FinishedLoad(bool success) {
	//Notify race state that the loading finished
	getRaceState()->onLoaded();
}

void ClientLoader::UpdatedLoadProgress(Ogre::Real progress) {
	//TODO notify server??
}

void ClientLoader::onSceneData(const OgreMax::Version& formatVersion, const OgreMax::Version& minOgreVersion, const OgreMax::Version& ogreMaxVersion, const Ogre::String& author, const Ogre::String& application, OgreMax::Types::UpAxis upAxis, Ogre::Real unitsPerMeter, const Ogre::String& unitType) {
	// We do nothing with this
}

void ClientLoader::onSkyBox(OgreMax::Types::SkyBoxParameters& skyBoxParameters, std::vector<OgreMax::Types::NodeAnimation> * animation ) {
	// Not yet supported
}

void ClientLoader::onSkyDome(OgreMax::Types::SkyDomeParameters& skyDomeParameters, std::vector<OgreMax::Types::NodeAnimation> * animation) {
	// Not yet supported
}

void ClientLoader::onClipping(Ogre::Real environmentNear, Ogre::Real environmentFar) {
	
}

void ClientLoader::onAmbientColour(const Ogre::ColourValue& colour) {
	// Just set the value of the colour of the ambient light
	mSceneMgr->setAmbientLight(colour);	
}

void ClientLoader::onShadowProperties(OgreMax::Types::ShadowParameters& params) {
  //Set the shadow parameters
	if (params.shadowTechnique == Ogre::SHADOWTYPE_NONE) {
        //Turn off shadows
        this->mSceneMgr->setShadowTechnique(Ogre::SHADOWTYPE_NONE);
    }
    else
    {
        //Turn on shadows
		this->mSceneMgr->setShadowTechnique(params.shadowTechnique);
        this->mSceneMgr->setShadowTextureSelfShadow(params.selfShadow);
        this->mSceneMgr->setShadowColour(params.shadowColor);
		if (params.farDistance > 0){
			this->mSceneMgr->setShadowFarDistance(params.farDistance);
		}

		//Set shadow texture parameters if necessary
        if (this->mSceneMgr->isShadowTechniqueTextureBased())
        {
			Ogre::RenderSystem* renderSystem = Ogre::Root::getSingleton().getRenderSystem();

            //Determine texture size
          /*  if (!renderSystem->getCapabilities()->hasCapability(Ogre::RSC_HWRENDER_TO_TEXTURE))
            {
                //Render to texture not supported, so ensure the shadow texture
                //size doesn't exceed the window size

                //Take minimum render window dimension as window size
				

                int windowSize = (int)std::min(this->mViewport->getWidth(), this->mViewport->getHeight());

                //Use the lesser of the texture and window sizes
                params.textureSize = std::min(params.textureSize, windowSize);
            }

            //If necessary, make sure the texture size is a power of two
			if (!OgreMax::OgreMaxUtilities::IsPowerOfTwo(params.textureSize) && !renderSystem->getCapabilities()->hasCapability(Ogre::RSC_NON_POWER_OF_2_TEXTURES)) {
				params.textureSize = OgreMax::OgreMaxUtilities::NextSmallestPowerOfTwo(params.textureSize);
            }*/

            if (params.pixelFormat == Ogre::PF_UNKNOWN) {
                //Choose a default format
				if (renderSystem->getName().find("GL") != Ogre::String::npos)
                {
	                //OpenGL performs better with a half-float format
	                params.pixelFormat = Ogre::PF_FLOAT16_R;
                }
                else
                {
	                //D3D is the opposite - if you ask for PF_FLOAT16_R you
	                //get an integer format instead. You can ask for PF_FLOAT16_GR
	                //but the precision doesn't work well
                    params.pixelFormat = Ogre::PF_FLOAT32_R;
                }
            }

            //Set texture size, count, pixel format
            this->mSceneMgr->setShadowTextureSettings(params.textureSize, params.textureCount, params.pixelFormat);

            //Set other texture settings
            this->mSceneMgr->setShadowDirLightTextureOffset(params.textureOffset);
            this->mSceneMgr->setShadowTextureFadeStart(params.textureFadeStart);
            this->mSceneMgr->setShadowTextureFadeEnd(params.textureFadeEnd);
            this->mSceneMgr->setShadowTextureCasterMaterial(params.textureShadowCasterMaterial);
            this->mSceneMgr->setShadowTextureReceiverMaterial(params.textureShadowReceiverMaterial);
        }

		 //Set shadow camera setup
        Ogre::ShadowCameraSetupPtr shadowCameraSetupPtr;
        if (!params.cameraSetup.empty()) {
			Ogre::String typeLower = params.cameraSetup;
			Ogre::StringUtil::toLowerCase(typeLower);

			if (typeLower == "uniform") {
				shadowCameraSetupPtr = Ogre::ShadowCameraSetupPtr(new Ogre::DefaultShadowCameraSetup());
			}
			else if (typeLower == "uniformfocused") {
				shadowCameraSetupPtr = Ogre::ShadowCameraSetupPtr(new Ogre::FocusedShadowCameraSetup());
			}
			else if (typeLower == "lispsm") {
				shadowCameraSetupPtr = Ogre::ShadowCameraSetupPtr(new Ogre::LiSPSMShadowCameraSetup());
			}
			else if (typeLower == "planeoptimal")
			{
				shadowCameraSetupPtr = Ogre::ShadowCameraSetupPtr(new Ogre::PlaneOptimalShadowCameraSetup(new Ogre::MovablePlane(params.optimalPlane)));
			} 
			else {
				Ogre::StringUtil::StrStreamType errorMessage;
				errorMessage << "Invalid shadow camera setup specified: " << typeLower;
				OGRE_EXCEPT
					(
					Ogre::Exception::ERR_INVALIDPARAMS,
					errorMessage.str(),
					"OgreMaxScene::ParseShadowCameraSetup"
					);
			}
        }
        else {
            //Create the appropriate default setup
			shadowCameraSetupPtr = Ogre::ShadowCameraSetupPtr(new Ogre::DefaultShadowCameraSetup());
        }
	    this->mSceneMgr->setShadowCameraSetup(shadowCameraSetupPtr);
	}
}

void ClientLoader::onBackgroundColour(const Ogre::ColourValue& colour) {
	// Not yet(?) supported
}

void ClientLoader::onLight(OgreMax::Types::LightParameters& parameters, const OgreMax::Types::Attachable * parent) {
	//Create the light
	Ogre::Light* light = mSceneMgr->createLight(parameters.name);
	if (parameters.queryFlags != 0)
		light->setQueryFlags(parameters.queryFlags);
	if (parameters.visibilityFlags != 0)
		light->setVisibilityFlags(parameters.visibilityFlags);

	light->setType(parameters.lightType);
	light->setCastShadows(parameters.castShadows);
	light->setPowerScale(parameters.power);
	light->setDiffuseColour(parameters.diffuseColor);
	light->setSpecularColour(parameters.specularColor);
	light->setPosition(parameters.position);
	light->setSpotlightFalloff(parameters.spotlightFalloff);
	light->setSpotlightInnerAngle(parameters.spotlightInnerAngle);
	light->setSpotlightOuterAngle(parameters.spotlightOuterAngle);
	light->setAttenuation(parameters.attenuationRange, parameters.attenuationConstant, parameters.attenuationLinear, parameters.attenuationQuadric);

	//Get Ogre parent
	Ogre::SceneNode* parentnode = 0;

	if ( parent == 0 ){
		parentnode = mSceneMgr->getRootSceneNode();
	} else {
		if ( mSceneMgr->hasSceneNode(parent->name) )
			parentnode =  mSceneMgr->getSceneNode(parent->name);
		else 
			parentnode = mSceneMgr->getRootSceneNode();
	}

	parentnode->attachObject(light);

}
    
void ClientLoader::onCamera(OgreMax::Types::CameraParameters& params, const OgreMax::Types::Attachable * parent) {
	Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "Position camera: " << params.position << "\n";

	// These settings only effect the free roaming camera in all game views and not the tracking cameras
	std::vector<GameView *> gameViews = RepresentationManager::getSingletonPtr()->getGameViews();
	for (std::vector<GameView *>::const_iterator it = gameViews.begin(); it != gameViews.end(); it++) {
		// Get camera and apply modifications to free roaming camera
		RaceCamera * cam = (*it)->getCamera();

		// TODO Currently FOV, clipping distances, etc ... are ignored by the parser. This could be applied in a later stage.
		if (mCurrNode) {
			// Use the parent position and orientation
			cam->setFreeroam(mCurrNode->getPosition(), mCurrNode->getOrientation());
		} else {
			// Use the camera position and orientation
			cam->setFreeroam(params.position, params.orientation, params.direction);
		}
	}
}

void ClientLoader::onEntity(OgreMax::Types::EntityParameters& entityparameters, const OgreMax::Types::Attachable * parent) {
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
			Entity * ent = EntityManager::getClientSingletonPtr()->getEntity(entityName);
			if (ent) {
				// Update task
				ProgressMonitor::getSingletonPtr()->updateTask("Loading world: " + ent->getCategory());
				EntityRepresentation * entRep = 0;
				bool visible = !(entityparameters.visibility == OgreMax::Types::OBJECT_HIDDEN);
				if (ent->getCategory() == Asteroid::CATEGORY) {
					// Create asteroid representation
					entRep = new AsteroidRepresentation(dynamic_cast<Asteroid *>(ent), mSceneMgr, entityparameters.meshFile, entityparameters.resourceGroupName, visible, entityparameters.castShadows, 
						entityparameters.renderingDistance, entityparameters.materialFile, subMaterials);
				} else if (ent->getCategory() == SpeedBoost::CATEGORY) {
					// Create boost representation
					entRep = new BoostRepresentation(dynamic_cast<SpeedBoost *>(ent), mSceneMgr, entityparameters.meshFile, entityparameters.resourceGroupName, visible, entityparameters.castShadows, 
						entityparameters.renderingDistance, entityparameters.materialFile, subMaterials);
				} else if (ent->getCategory() == CheckPoint::CATEGORY) {
					// Create checkpoint representation
					entRep = new CheckPointRepresentation(dynamic_cast<CheckPoint *>(ent), mSceneMgr, entityparameters.meshFile, entityparameters.resourceGroupName, visible, entityparameters.castShadows, 
						entityparameters.renderingDistance, entityparameters.materialFile, subMaterials);
				} else if (ent->getCategory() == Finish::CATEGORY) {
					// Create finish representation
					entRep = new FinishRepresentation(dynamic_cast<Finish *>(ent), mSceneMgr, entityparameters.meshFile, entityparameters.resourceGroupName, visible, entityparameters.castShadows, 
						entityparameters.renderingDistance, entityparameters.materialFile, subMaterials);
				} else if (ent->getCategory() == Hovercraft::CATEGORY) {
					// Create hovercraft representation
					entRep = new HovercraftRepresentation(dynamic_cast<Hovercraft *>(ent), mSceneMgr, entityparameters.meshFile, entityparameters.resourceGroupName, visible, entityparameters.castShadows, 
						entityparameters.renderingDistance, entityparameters.materialFile, subMaterials);
				} else if (ent->getCategory() == Portal::CATEGORY) {
					// Create portal representation
					entRep = new PortalRepresentation(dynamic_cast<Portal *>(ent), mSceneMgr, entityparameters.meshFile, entityparameters.resourceGroupName, visible, entityparameters.castShadows, 
						entityparameters.renderingDistance, entityparameters.materialFile, subMaterials);
				} else if (ent->getCategory() == Start::CATEGORY) {
					// Create start representation
					entRep = new StartRepresentation(dynamic_cast<Start *>(ent), mSceneMgr, entityparameters.meshFile, entityparameters.resourceGroupName, visible, entityparameters.castShadows, 
						entityparameters.renderingDistance, entityparameters.materialFile, subMaterials);
				} else if (ent->getCategory() == StaticBody::CATEGORY) {
					// Create start representation
					entRep = new StaticBodyRepresentation(dynamic_cast<StaticBody *>(ent), mSceneMgr, entityparameters.meshFile, entityparameters.resourceGroupName, visible, entityparameters.castShadows, 
						entityparameters.renderingDistance, entityparameters.materialFile, subMaterials);
				}
				
				
				
				/*else if (ent->getCategory() == Track::CATEGORY) {
					// Create track representation
					entRep = new TrackRepresentation(dynamic_cast<Track *>(ent), mSceneMgr, entityparameters.meshFile, entityparameters.resourceGroupName, visible, entityparameters.castShadows, 
						entityparameters.renderingDistance, entityparameters.materialFile, subMaterials);
				}*/ else {
					// Unrecognized entity, ignore
				}

				if (entRep) {
					// Add to representation manager
					RepresentationManager::getSingletonPtr()->addEntityRepresentation(entRep);
				}
			} else {
				// The entity did not exist
				Ogre::Log::Stream stream = Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream();
				stream << "Entity with name |" << entityName << "| does not exist in client." << "\n";

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

void ClientLoader::onNode(OgreMax::Types::NodeParameters& nodeparameters, std::vector<OgreMax::Types::NodeAnimation> * animation, const OgreMax::Types::NodeParameters* parent) {
	Ogre::String name = nodeparameters.name;
	
	// Create node
	Ogre::SceneNode* node = mSceneMgr->getRootSceneNode()->createChildSceneNode(name);

	// Set properties
	node->setPosition(nodeparameters.position);
	node->setOrientation(nodeparameters.orientation);
	node->setScale(nodeparameters.scale);

	// Store node as current node
	mCurrNode = node;
}

}