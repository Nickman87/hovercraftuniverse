#include "Moveable3DEmitter.h"

#include "SoundManager.h"

#include <OgreRoot.h>

namespace HovUni {
	const Ogre::String Moveable3DEmitter::msPrefix = "Moveable3DEmitter :: ";

	Moveable3DEmitter::Moveable3DEmitter(const char* eventGUID) : mEventGUID(eventGUID), mShouldPlay(false) {
		//Try and get an instance of this event
		handleError(SoundManager::getSingletonPtr()->getSoundEvent(mEventGUID, &mEvent), CONSTRUCTOR);
	}

	Moveable3DEmitter::~Moveable3DEmitter() {
		if (mEvent != 0) {
			mEvent->release();
		}
	}

	void Moveable3DEmitter::update() {
		//Update everything we need
		if (mShouldPlay) {
			//Make sure we are playing
			startSound();

			//Get updated location
			Ogre::Vector3 * position;
			Ogre::Vector3 * velocity;
			Ogre::Vector3 * orientation;

			getUpdates(&position, &velocity, &orientation);
			
			//Update the position, velocity and orientation
			FMOD_VECTOR pos;
			FMOD_VECTOR vel;
			FMOD_VECTOR ori;
			
			if (position != 0) {
				pos.x = position->x;
				pos.y = position->y;
				pos.z = position->z;
			}

			if (velocity != 0) {
				vel.x = velocity->x;
				vel.y = velocity->y;
				vel.z = velocity->z;
			}

			if (orientation != 0) {
				ori.x = orientation->x;
				ori.y = orientation->y;
				ori.z = orientation->z;
			}

			

			if (position != 0) {
				//Ogre::LogManager::getSingleton().getDefaultLog()->stream() << msPrefix << "Updating position to: " << pos.x << ", " << pos.y << ", " << pos.z;
			} else {
				Ogre::LogManager::getSingleton().getDefaultLog()->stream() << msPrefix << "got null for new position vector";
			}

			mEvent->set3DAttributes(position ? &pos : 0, velocity ? &vel : 0, orientation ? &ori : 0);
			mEvent->get3DAttributes(&pos, &vel, &ori);

			//Ogre::LogManager::getSingleton().getDefaultLog()->stream() << msPrefix << "New position is: " << pos.x << ", " << pos.y << ", " << pos.z;

			//Make sure we are playing
			startSound();
		} else {
			//We shouldn't be playing, make sure the music is stopped
			stopSound();
		}
	}

	bool Moveable3DEmitter::handleError(FMOD_RESULT result, Origin orig) {
		if (result != FMOD_OK) {
			//TODO: filter errors correctly
			Ogre::LogManager::getSingleton().getDefaultLog()->stream() << msPrefix << "ERROR: " << result << " " << FMOD_ErrorString(result) << " origin(" << orig << ")";
			return false;
		} else {
			return true;
		}
	}

	void Moveable3DEmitter::startSound() {
		mShouldPlay = true;

		//Start playing music
		if (mEvent == 0) {
			//Try to get a new event
			std::cout << "Starting engine music" << std::endl;
			if (!handleError(SoundManager::getSingletonPtr()->getSoundEvent(mEventGUID, &mEvent), PLAY)) {
				//Could not play... we tried
				std::cout << "Starting engine music FAILED" << std::endl;
				return;
			}
		}

		//We should have a valid event, start playing
		handleError(mEvent->start(), PLAY);
	}

	void Moveable3DEmitter::setEventParameter(unsigned int parameterID, float value) {
		FMOD::EventParameter* param;
		mEvent->getParameterByIndex(parameterID, &param);
		
		param->setValue(value);
	}

	void Moveable3DEmitter::getEventParameterRange(unsigned int parameterID, float* min, float* max) {
		FMOD::EventParameter* param;
		mEvent->getParameterByIndex(parameterID, &param);

		param->getRange(min, max);
	}

	void Moveable3DEmitter::stopSound() {
		mShouldPlay = false;

		//Stop playing music
		if (mEvent != 0) {
			handleError(mEvent->stop(), STOP);
		}
	}

	void Moveable3DEmitter::setVolume(float volume) {
		if (mEvent != 0) {
			mEvent->setVolume(volume);
		}
	}
}