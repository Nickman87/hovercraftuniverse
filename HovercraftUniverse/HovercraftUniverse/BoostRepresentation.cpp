#include "BoostRepresentation.h"
#include "SpeedBoost.h"

namespace HovUni {

BoostRepresentation::BoostRepresentation(SpeedBoost * entity, Ogre::SceneManager * sceneMgr, Ogre::String meshFile, Ogre::String resourceGroupName, bool visible, bool castShadows, 
										 Ogre::Real renderingDistance, Ogre::String materialFile, std::vector<Ogre::String> subMaterials) 
										: EntityRepresentation(entity, meshFile,  sceneMgr, resourceGroupName, visible, castShadows, renderingDistance, materialFile, subMaterials) {
	// Empty
}

BoostRepresentation::~BoostRepresentation() {
	// Empty
}

}