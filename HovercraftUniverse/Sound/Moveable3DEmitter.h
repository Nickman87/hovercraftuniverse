#ifndef MOVEABLE3DEMITTER_H
#define MOVEABLE3DEMITTER_H

#include <OgreVector3.h>
#include <fmod_event.hpp>

namespace HovUni {
	/**
	 * A virtual class representing a moveable 3D sound emitter.
	 * Objects that want to emmit sound in a three dimensional space
	 * must extend this class and implement missing methods.
	 *
	 * @author Nick De Frangh
	 */
	class Moveable3DEmitter {
		private:
			/** The unique ID to identify the corresponding event */
			const char* mEventGUID;

			/** Constant prefix for output */
			static const Ogre::String msPrefix;

			/** Enum to identify different source of error checking */
			enum Origin {
				CONSTRUCTOR = 0, UPDATE, PLAY, STOP
			};

			/** 
			 * A pointer to the event linked to this emitter.
			 * This pointer can be 0 if the event pool has maximised.
			 * This can indicate that you should try to start playing the sound
			 * again.
			 */
			FMOD::Event* mEvent;

			bool mShouldPlay;

		public:
			/**
			 * Basic constructor for a Moveable3DEmitter.
			 *
			 * @param eventGUID The ID of the event linked to the objects' sound.
			 */
			Moveable3DEmitter(const char* eventGUID);

			virtual ~Moveable3DEmitter();

			/**
			 * Update function that will be called every iteration. This will check if we should
			 * still be playing, update the position, etc.
			 */
			void update();

		protected:
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
			virtual void getUpdates(Ogre::Vector3 ** position, Ogre::Vector3 ** velocity, Ogre::Vector3 ** orientation) = 0;

			/**
			 * function to handle all possible errors that might occur and act
			 * accordingly. This will be used to monitor full playback listst so
			 * we can try making noise again next iteration.
			 *
			 * @param result The result you want to check for errors
			 * @param orig The origin of the error
			 * @return True if all when OK, false otherwise
			 */
			bool handleError(FMOD_RESULT result, Origin orig);

			/**
			 * Set a parameter on the current event.
			 *
			 * @param parameterID The id of the parameter you want to set
			 * @param vlaue The value you want to set the perameter to
			 */
			void setEventParameter(unsigned int parameterID, float value);

			/**
			 * Request the range of a certain parameter.
			 *
			 * @param parameterID The id of the parameter you want to set
			 * @param min The minimum value of the parameter will be stored here
			 * @param max The maximum value of the parameter will be stored here
			 */
			void getEventParameterRange(unsigned int parameterID, float* min, float* max);

		public:
			/**
			 * Start making some noise.
			 */
			void startSound();

			/**
			 * Stop making noise.
			 */
			void stopSound();

			/**
			 * Set the volume of the current emitter
			 *
			 * @param volume The colume value
			 */
			void setVolume(float volume);
	};
}

#endif //MOVEABLE3DEMITTER_H