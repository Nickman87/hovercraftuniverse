#ifndef ASTEROIDREPRESENTATION_H_
#define ASTEROIDREPRESENTATION_H_

#include "EntityRepresentation.h"

namespace HovUni {

	class Asteroid;

	/**
	 * Representation of an asteroid entity.
	 *
	 * @author Kristof Overdulve
	 */
	class AsteroidRepresentation : public EntityRepresentation
	{
	public:
		/**
		 * Constructor.
		 */
		AsteroidRepresentation(Asteroid * entity, Ogre::SceneManager * sceneMgr, Ogre::String meshFile, Ogre::String resourceGroupName, bool visible, bool castShadows, 
			Ogre::Real renderingDistance, Ogre::String materialFile, std::vector<Ogre::String> subMaterials);

		/**
		 * Destructor.
		 */
		~AsteroidRepresentation(void);
	};

}

#endif