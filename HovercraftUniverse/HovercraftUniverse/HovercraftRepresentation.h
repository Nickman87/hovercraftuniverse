#ifndef HOVERCRAFTREPRESENTATION_H_
#define HOVERCRAFTREPRESENTATION_H_

#include "EntityRepresentation.h"
#include <vector>

#include <Moveable3DEmitter.h>


namespace HovUni {

	class Hovercraft;

	/**
	 * Representation of a hovercraft entity.
	 *
	 * @author Kristof Overdulve, Nick De Frangh, Dirk Delahaye
	 */
	class HovercraftRepresentation : public EntityRepresentation, public Moveable3DEmitter {
	private:
		static const float MAX_RPM;
		static const float MIN_RPM;

		/** The Particle Scene node, must be updated on every step */
		Ogre::SceneNode* mParticleNode;

		/** Animation state for rotation of hover */
		Ogre::AnimationState * mRotorState;

		/** Collision Sparks */
		Ogre::ParticleSystem* mSparks;
		Ogre::SceneNode* mSparksNode;
	public:
		/**
		 * Constructor.
		 */
		HovercraftRepresentation(Hovercraft * entity, Ogre::SceneManager * sceneMgr, Ogre::String meshFile, Ogre::String resourceGroupName, bool visible, bool castShadows, 
			Ogre::Real renderingDistance, Ogre::String materialFile, std::vector<Ogre::String> subMaterials, Ogre::SceneNode * node = 0);

		/**
		 * Destructor.
		 */
		~HovercraftRepresentation();

		/**
		 * Draws the representation.
		 * 
		 * @param time since last frame
		 */
		void draw( Ogre::Real timeSinceLastFrame );

		/**
		 * This function will be called every cycle to get updates for the position,
		 * velocity and orientation. This is needed to calculate the 3D sound in a
		 * realistic way.
		 *
		 * @param position Position in 3D space of the event. Specifying 0 / null will ignore this parameter. 
		 * @param velocity Velocity in 'distance units per second' in 3D space of the event. See remarks.
		 *		Specifying 0 / null will ignore this parameter
		 * @param orientation Orientation of the event sound cone.
		 *		Only used if the event has a cone specified to determine cone detection, otherwise just 
		 *		specify 0 / null. Specifying 0 / null will ignore this parameter.
		 */
		virtual void getUpdates(Ogre::Vector3 ** position, Ogre::Vector3 ** velocity, Ogre::Vector3 ** orientation);
	};

}

#endif