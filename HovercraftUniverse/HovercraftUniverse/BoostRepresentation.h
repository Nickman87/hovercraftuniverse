#ifndef BOOSTREPRESENTATION_H_
#define BOOSTREPRESENTATION_H_

#include "EntityRepresentation.h"
#include <vector>

namespace HovUni {

	class SpeedBoost;

	/**
	 * Representation of a boost entity.
	 *
	 * @author Kristof Overdulve
	 */
	class BoostRepresentation : public EntityRepresentation
	{
	public:
		/**
		 * Constructor.
		 */
		BoostRepresentation(SpeedBoost * entity, Ogre::SceneManager * sceneMgr, Ogre::String meshFile, Ogre::String resourceGroupName, bool visible, bool castShadows, 
			Ogre::Real renderingDistance, Ogre::String materialFile, std::vector<Ogre::String> subMaterials);

		/**
		 * Destructor.
		 */
		~BoostRepresentation();
	};

}

#endif