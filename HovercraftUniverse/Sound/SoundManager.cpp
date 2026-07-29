#include "SoundManager.h"

#include <OgreRoot.h>

namespace HovUni {
	SoundManager* SoundManager::msSingleton = 0;
	const Ogre::String SoundManager::msPrefix = "SoundManager :: ";

	SoundManager::SoundManager(const Ogre::String& mediaPath, const Ogre::String& mediaFile) {
		//Initialize some stuff
		ERRCHECK(mResult = FMOD::EventSystem_Create(&mEventSystem));
		ERRCHECK(mResult = mEventSystem->getSystemObject(&mSystem));		
		ERRCHECK(mResult = mSystem->setGeometrySettings(200.0f));

		int numdrivers;
		ERRCHECK(mResult = mSystem->getNumDrivers(&numdrivers));

		if (numdrivers == 0) {
			//We have no soundcard...
			ERRCHECK(mResult = mSystem->setOutput(FMOD_OUTPUTTYPE_NOSOUND));
		} else {
			FMOD_SPEAKERMODE speakermode;
			FMOD_CAPS caps;
			char name[256];

			ERRCHECK(mResult = mSystem->getDriverCaps(0, &caps, 0, 0, &speakermode));
			ERRCHECK(mResult = mSystem->setSpeakerMode(speakermode));       /* Set the user selected speaker mode. */

			if (caps & FMOD_CAPS_HARDWARE_EMULATED) {             /* The user has the 'Acceleration' slider set to off!  This is really bad for latency!. */
				ERRCHECK(mResult = mSystem->setDSPBufferSize(1024, 10));/* You might want to warn the user about this. */
			}

			ERRCHECK(mResult = mSystem->getDriverInfo(0, name, 256, 0));
			if (strstr(name, "SigmaTel")) {  /* Sigmatel sound devices crackle for some reason if the format is PCM 16bit.  PCM floating point output seems to solve it. */
				ERRCHECK(mResult = mSystem->setSoftwareFormat(48000, FMOD_SOUND_FORMAT_PCMFLOAT, 0,0, FMOD_DSP_RESAMPLER_LINEAR)); 
			}
		}

		mResult = mEventSystem->init(256, FMOD_INIT_3D_RIGHTHANDED, 0, FMOD_EVENT_INIT_USE_GUIDS);

		if (mResult == FMOD_ERR_OUTPUT_CREATEBUFFER) {        /* Ok, the speaker mode selected isn't supported by this soundcard.  Switch it back to stereo... */
			ERRCHECK(mResult = mSystem->setSpeakerMode(FMOD_SPEAKERMODE_STEREO));

			ERRCHECK(mResult = mEventSystem->init(256, FMOD_INIT_3D_RIGHTHANDED, 0, FMOD_EVENT_INIT_USE_GUIDS));/* ... and re-init. */
		}

		//Data initialisation
		ERRCHECK(mResult = mEventSystem->setMediaPath(mediaPath.c_str()));

		//Load a sound object
		Ogre::LogManager::getSingleton().getDefaultLog()->stream() << msPrefix << "Loading mediafile : " << mediaPath.c_str() << mediaFile.c_str();
		ERRCHECK(mResult = mEventSystem->load(mediaFile.c_str(), 0, 0));
		Ogre::LogManager::getSingleton().getDefaultLog()->stream() << msPrefix << "Mediafile loaded.";

		ERRCHECK(mResult = mEventSystem->getMusicSystem(&mMusicSystem));
	}
	
	FMOD_RESULT SoundManager::getSoundEvent(const char* eventGUID, FMOD::Event ** evt) {
		return mEventSystem->getEventByGUIDString(eventGUID, FMOD_EVENT_DEFAULT, evt);
	}

	SoundManager& SoundManager::getSingleton(void) {
		if (msSingleton == 0) {
			//TODO: throw exception
		}

		return *msSingleton;
	}

	SoundManager* SoundManager::getSingletonPtr(void) {
		if (msSingleton == 0) {
			//TODO: throw exception
		}

		return msSingleton;
	}

	void SoundManager::init(const Ogre::String& mediaPath, const Ogre::String& mediaFile) {
		msSingleton = new SoundManager(mediaPath, mediaFile);
	}

	SoundManager::~SoundManager() {
		//Clean up
		ERRCHECK(mResult = mEventSystem->release());
	}

	void SoundManager::ERRCHECK(FMOD_RESULT result) {
		if (result != FMOD_OK) {
			Ogre::LogManager::getSingleton().getDefaultLog()->stream() << msPrefix << "ERROR: " << result << " " << FMOD_ErrorString(result);
			assert(false);
		}
	}

	int SoundManager::start3DSound(int soundId, const Ogre::Vector3& position, const Ogre::Vector3& velocity) {
		return 0;
	}

	void SoundManager::stop3DSound(int channelId) {
		
	}

	void SoundManager::startAmbient(unsigned int ambientId) {
		//Lets see if this ambient already exists
		if (mPrompts.find(ambientId) == mPrompts.end()) {
			//Item does not exist, create it
			FMOD::MusicPrompt* newMusic;

			ERRCHECK(mResult = mMusicSystem->prepareCue(ambientId, &newMusic));

			mPrompts[ambientId] = newMusic;
		}

		//Actually start the ambient
		ERRCHECK(mResult = mPrompts[ambientId]->begin());
	}

	void SoundManager::stopAmbient(unsigned int ambientId) {
		if (mPrompts.find(ambientId) == mPrompts.end()) {
			//Does not exist...
		} else {
			ERRCHECK(mResult = mPrompts[ambientId]->end());
		}
	}

	void SoundManager::updateListenerPosition(Ogre::Vector3* position, Ogre::Vector3* velocity, Ogre::Vector3* forward, Ogre::Vector3* up) {
		FMOD_VECTOR pos;
		FMOD_VECTOR vel;
		FMOD_VECTOR forw;
		FMOD_VECTOR upF;

		if (position) {
			pos.x = position->x;
			pos.y = position->y;
			pos.z = position->z;
		}
		if (velocity) {
			vel.x = velocity->x;
			vel.y = velocity->y;
			vel.z = velocity->z;
		}
		if (forward) {
			forw.x = forward->x;
			forw.y = forward->y;
			forw.z = forward->z;
		}
		if (up) {
			upF.x = up->x;
			upF.y = up->y;
			upF.z = up->z;
		}
/*
		if (position != 0) {
			Ogre::LogManager::getSingleton().getDefaultLog()->stream() << msPrefix << "Updating LISTENER position to: " << pos.x << ", " << pos.y << ", " << pos.z;
		} else {
			Ogre::LogManager::getSingleton().getDefaultLog()->stream() << msPrefix << "got null for new LISTENER position vector";
		}
*/		
		ERRCHECK(mResult = mEventSystem->set3DListenerAttributes(0, position ? &pos : 0, velocity ? &vel : 0, forward ? &forw : 0, up ? &upF : 0));

		mEventSystem->get3DListenerAttributes(0, &pos, &vel, &forw, &upF);
		//Ogre::LogManager::getSingleton().getDefaultLog()->stream() << msPrefix << "New LISTENER position is: " << pos.x << ", " << pos.y << ", " << pos.z;
	}

	void SoundManager::setEventParameter(const char* eventGUID, unsigned int parameterID, float value) {
		FMOD::Event* currEvent;
		ERRCHECK(mResult = mEventSystem->getEventByGUIDString(eventGUID, FMOD_EVENT_DEFAULT, &currEvent));

		
		
		FMOD::EventParameter* param;
		ERRCHECK(mResult = currEvent->getParameterByIndex(parameterID, &param));
		ERRCHECK(mResult = param->setValue(value));
	}

	void SoundManager::getEventParameterRange(const char* eventGUID, unsigned int parameterID, float* min, float* max) {
		FMOD::Event* currEvent;
		ERRCHECK(mResult = mEventSystem->getEventByGUIDString(eventGUID, FMOD_EVENT_DEFAULT, &currEvent));

		FMOD::EventParameter* param;
		ERRCHECK(mResult = currEvent->getParameterByIndex(parameterID, &param));
		ERRCHECK(mResult = param->getRange(min, max));
	}

	void SoundManager::registerEmitter(Moveable3DEmitter* emitter) {
		for (std::vector<Moveable3DEmitter*>::iterator it = mEmitters.begin(); it != mEmitters.end(); ++it) {
			if ( (*it) == emitter ) {
				//Already exists
				return;
			}
		}

		mEmitters.push_back(emitter);
	}

	void SoundManager::deregisterEmitter(Moveable3DEmitter* emitter) {
		for (std::vector<Moveable3DEmitter*>::iterator it = mEmitters.begin(); it != mEmitters.end(); ++it) {
			if ( (*it) == emitter ) {
				mEmitters.erase(it);
				return;
			}
		}
	}

	void SoundManager::update() {
		//Update all the emitters
		for (std::vector<Moveable3DEmitter*>::iterator it = mEmitters.begin(); it != mEmitters.end(); ++it) {
			(*it)->update();
		}

		ERRCHECK(mResult = mEventSystem->update());
	}

}