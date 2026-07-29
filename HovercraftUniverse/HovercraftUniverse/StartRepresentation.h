#ifndef STARTREPRESENTATION_H_
#define STARTREPRESENTATION_H_

#include "EntityRepresentation.h"
#include <vector>

namespace HovUni {

	class Start;

	/**
	 * Representation of a start entity.
	 *
	 * @author Kristof Overdulve
	 */
	class StartRepresentation : public EntityRepresentation
	{
	public:
		/**
		 * Constructor.
		 */
		StartRepresentation(Start * entity, Ogre::SceneManager * sceneMgr, Ogre::String meshFile, Ogre::String resourceGroupName, bool visible, bool castShadows, 
			Ogre::Real renderingDistance, Ogre::String materialFile, std::vector<Ogre::String> subMaterials);

		/**
		 * Destructor.
		 */
		~StartRepresentation();
	};

}

#endif