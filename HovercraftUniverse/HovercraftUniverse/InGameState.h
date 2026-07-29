#ifndef INGAMESTATE_H
#define INGAMESTATE_H

#include "BasicGameState.h"
#include "Countdown.h"
#include "HUClient.h"
#include "EntityManager.h"
#include "HUD.h"
#include "LoadingOverlay.h"
#include "ProgressMonitorListener.h"
#include "RaceStateListener.h"
#include "RepresentationManager.h"
#include "Results.h"
#include <tinyxml/tinyxml.h>
#include <SkyX.h>
#include <boost/thread.hpp>
//#include <boost/detail/mutex.hpp>

namespace HovUni {
	class RaceState; //Forward declaration

	/**
	 * A class that represents the ingame state.
	 * This state will take care of passing the right inputs to the right locations,
	 * managing the hud, managing the rendering scene, ...
	 *
	 * @auhtor Nick De Frangh
	 */
	class InGameState : public BasicGameState, public RaceStateListener, public IProgressMonitorListener {
		private:
			/** A reference to the created client core */
			HUClient* mHUClient;

			/** The instantiation of the hud */
			HUD* mHud;

			/** Monitor the time lapsed for the HUClient */
			float mTimeLapsed;

			/** A reference to the entity manager */
			EntityManager* mEntityManager;

			/** A reference to the representation manager */
			RepresentationManager* mRepresentationManager;

			/** A boolean to mark when we want to quit */
			bool mContinue;

			/** The race state associated with this race */
			RaceState* mRaceState;

			/** The loading overlay we will be displaying */
			LoadingOverlay* mLoader;

			/** A countdown overlay */
			Countdown* mCountdown;

			/** Results overlay */
			Results* mResults;

			/** Value to keep the fadeout time for the countdown overlay */
			Ogre::Real mCountdownFadeout;

			/** Boolean to check if we should update the listener position or not */
			bool mUpdateListener;

			/** The sky renderer */
			SkyX::SkyX * mSkyX;

			/** Boolean to check if we are cleaning up */
			bool mCleaningUp;

			/** Whether to run */
			bool mRun;

			/** Mutex */
			boost::mutex mMutex;

			/** Boolean to check for finish */
			bool mFinished;

		public:
			/**
			 * Constructor
			 *
			 * @param client The client to communicate with the server
			 * @param raceState The racestate linked to this session
			 * @param HUDConfig The XML element representing the HUD configuration
			 */
			InGameState(HUClient* client, RaceState* raceState, TiXmlElement* HUDConfig);

			/**
			 * Function that will handle new Chat Input
			 */
			Hikari::FlashValue onChat(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			////////////////////////////////////////////
			//	 ProgressMonitorListener functions	  //
			////////////////////////////////////////////

			/**
			 * This function will be called when the loading has changed value
			 *
			 * @param progress the current progress after the update
			 */
			virtual void updateProgress(double progress);

			////////////////////////////////////////////
			//			RaceStateListener functions	  //
			////////////////////////////////////////////

			/**
			 * The state of the race changed
			 *
			 * @param state the new state
			 */
			virtual void onStateChange(RaceState::States state);

			/**
			 * The player changed position
			 *
			 * @param player the player
			 */
			virtual void onPositionChange(RacePlayer* player);

			/**
			 * The player finished
			 *
			 * @param player the player
			 */
			virtual void onFinish(RacePlayer* player);

			////////////////////////////////////////////
			//			BasicGameState functions	  //
			////////////////////////////////////////////

			/**
			 * Function called when the state gets activated
			 */
			virtual void activate();

			/**
			 * Function called when the state gets disabled
			 */
			virtual void disable();

			/**
			 * Function called every frame, do drawing functions and stuff in here
			 */
			virtual bool frameStarted(const Ogre::FrameEvent & evt);

			/**
			 * @see MouseListener::mouseMoved().
			 * Can be implemented as needed.
			 */
			virtual bool mouseMoved(const OIS::MouseEvent & e);

			/**
			 * @see MouseListener::mousePressed().
			 * Can be implemented as needed.
			 */
			virtual bool mousePressed(const OIS::MouseEvent & e, OIS::MouseButtonID id);
			
			/**
			 * @see MouseListener::mouseReleased().
			 * Can be implemented as needed.
			 */
			virtual bool mouseReleased(const OIS::MouseEvent & e, OIS::MouseButtonID id);

			/**
			 * @see KeyListener::keyPressed().
			 * Can be implemented as needed.
			 */
			virtual bool keyPressed(const OIS::KeyEvent & e);

			/**
			 * @see KeyListener::keyReleased().
			 * Can be implemented as needed.
			 */
			virtual bool keyReleased(const OIS::KeyEvent & e);

		protected:
			/**
			 * Call this function to update the hud visuals
			 */
			void updateHud();
	};
}

#endif //INGAMESTATE_H