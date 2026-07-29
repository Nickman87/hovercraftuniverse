#ifndef _CAMERASPRING_H
#define _CAMERASPRING_H

#include <Ogre.h>

namespace HovUni {
	/**
	 * A class to simulate the camera hanging from a string.
	 * Original author: Liam Rüdel - liamrudel@gmail.com
	 *		http://www.liamrudel.com/?page_id=423
	 * 
	 * @author Nick De Frangh
	 */
	class CameraSpring {
	public:
		/** Camera position */
		Ogre::Vector3 mCameraPos;

		/** Position we want to be at, idealy somewhere behind the player e.g.(0,50,-100); */
		Ogre::Vector3 mDesiredPosition;

		/** Cameras Velocity */
		Ogre::Vector3 mVelocity;

		/** Cameras Acceleration */
		Ogre::Vector3 mAcceleration;

		/** Cameras Velocity after dampening */
		Ogre::Vector3 mDampenedVelocity;

		/** Amount Camera Stretched */
		Ogre::Vector3 mStretch;

		/** Force applied to the camera */
		Ogre::Vector3 mForce;
		
		/** Spring Stiffness */
		float mStiffness;

		/** Spring Dampening */
		float mDampening;

		/** Spring Mass */
		float mMass;

		/** Default Constructor */
		CameraSpring();

		/**
		 * Sets up a camera spring
		 *
		 * @param camPos The initial camera position
		 * @param offsetPos The initial offset from the object you are tracking
		 */
		void initCameraSpring(Ogre::Vector3 camPos,Ogre::Vector3 offsetPos);

		/**
		 * Updates the camera position
		 *
		 * @param camPos The camera position you want to go to
		 * @param offsetPos The offset you want to go to
		 */
		Ogre::Vector3 updateCameraSpring(Ogre::Vector3 camPos,Ogre::Vector3 offsetPos);

		/**
		 * Request the mass
		 *
		 * @return The mass of the camera
		 */
		float getMass(){return mMass;}

		/**
		 * Request the dampening
		 *
		 * @return The dampening factor
		 */
		float getDampening(){return mDampening;}

		/**
		 * Request the stiffness
		 *
		 * @return The stiffness factor
		 */
		float getStiffness(){return mStiffness;}

		/**
		 * Set the mass
		 *
		 * @param mass the new mass
		 */
		void setMass(float mass){mMass = mass;}

		/**
		 * Set the dampening factor
		 *
		 * @param dampening The new dampening factor
		 */
		void setDampening(float dampening){mDampening = dampening;}

		/**
		 * Set the stiffness factor
		 *
		 * @param stiffness The new stiffness factor
		 */
		void setStiffness(float stiffness){mStiffness = stiffness;}

		/**
		 * Get a singleton instance of the cameraSpring
		 *
		 * @return A pointer tot he camera spring singleton
		 */
		static CameraSpring* getInstance() { return &mCameraSpring; }

	private:
		/** A static instance of a Cameraspring */
		static CameraSpring mCameraSpring;
		
	};
}

#endif //_CAMERASPRING_H