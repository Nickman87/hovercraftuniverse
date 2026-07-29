#ifndef PORTALREPRESENTATION_H_
#define PORTALREPRESENTATION_H_

#include "EntityRepresentation.h"
#include <vector>

namespace HovUni {

	class Portal;

	/**
	 * Representation of a portal entity.
	 *
	 * @author Kristof Overdulve
	 */
	class PortalRepresentation : public EntityRepresentation
	{
	public:
		/**
		 * Constructor.
		 */
		PortalRepresentation(Portal * entity, Ogre::SceneManager * sceneMgr, Ogre::String meshFile, Ogre::String resourceGroupName, bool visible, bool castShadows, 
			Ogre::Real renderingDistance, Ogre::String materialFile, std::vector<Ogre::String> subMaterials);

		/**
		 * Destructor.
		 */
		~PortalRepresentation();
	};

}

#endif