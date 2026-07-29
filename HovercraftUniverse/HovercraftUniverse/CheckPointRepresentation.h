#ifndef CHECKPOINTREPRESENTATION_H_
#define CHECKPOINTREPRESENTATION_H_

#include "EntityRepresentation.h"
#include <vector>

namespace HovUni {

	class CheckPoint;

	/**
	 * Representation of a checkpoint entity.
	 *
	 * @author Kristof Overdulve
	 */
	class CheckPointRepresentation : public EntityRepresentation
	{
	public:
		/**
		 * Constructor.
		 */
		CheckPointRepresentation(CheckPoint * entity, Ogre::SceneManager * sceneMgr, Ogre::String meshFile, Ogre::String resourceGroupName, bool visible, bool castShadows, 
			Ogre::Real renderingDistance, Ogre::String materialFile, std::vector<Ogre::String> subMaterials);

		/**
		 * Destructor.
		 */
		~CheckPointRepresentation();
	};

}

#endif