#ifndef STATICBODYREPRESENTATION_H_
#define STATICBODYREPRESENTATION_H_

#include "EntityRepresentation.h"
#include <vector>

namespace HovUni {

	class StaticBody;

	/**
	 * Representation of a RigidBody entity.
	 *
	 */
	class StaticBodyRepresentation : public EntityRepresentation
	{
	public:

		/**
		 * Constructor.
		 */
		StaticBodyRepresentation(StaticBody * entity, Ogre::SceneManager * sceneMgr, Ogre::String meshFile, Ogre::String resourceGroupName, bool visible, bool castShadows, 
			Ogre::Real renderingDistance, Ogre::String materialFile, std::vector<Ogre::String> subMaterials);

		/**
		 * Destructor.
		 */
		~StaticBodyRepresentation();
	};

}

#endif
