#ifndef BASICGAMESTATE_H
#define BASICGAMESTATE_H

#include "GameStateManager.h"
#include <GUIManager.h>
#include <SoundManager.h>
#include "InputManager.h"

namespace HovUni {
	/**
	 * A class to represent a game state. Specific states should be derived
	 * from this basic class
	 *
	 * @author Nick De Frangh
	 */
	class BasicGameState : public Ogre::FrameListener, public OIS::MouseListener, public OIS::KeyListener {
		protected:
			/** A reference to the GameStateManager */
			GameStateManager* mManager;

			/** A reference to the GUI Manager */
			GUIManager* mGUIManager;

			/** A reference to the Sound Manager */
			SoundManager* mSoundManager;

			/** A reference tot he Input manager */
			InputManager* mInputManager;

		public:
			/** Default constructor */
			BasicGameState();

			/**
			 * Basic functionality that will be executed on activation
			 */
			virtual void onActivate();

			/**
			 * FBasic functionality that will be executed on disable
			 */
			virtual void onDisable();

			/**
			 * Function called when the state gets activated
			 */
			virtual void activate() = 0;

			/**
			 * Function called when the state gets disabled
			 */
			virtual void disable() = 0;

			/**
			 * @see FrameListener::frameStarted().
			 * Must be implemented.
			 */
			virtual bool frameStarted(const Ogre::FrameEvent & evt) = 0;

			/**
			 * @see MouseListener::mouseMoved().
			 * Can be implemented as needed.
			 */
			virtual bool mouseMoved(const OIS::MouseEvent & e) { return true; }

			/**
			 * @see MouseListener::mousePressed().
			 * Can be implemented as needed.
			 */
			virtual bool mousePressed(const OIS::MouseEvent & e, OIS::MouseButtonID id) { return true; }
			
			/**
			 * @see MouseListener::mouseReleased().
			 * Can be implemented as needed.
			 */
			virtual bool mouseReleased(const OIS::MouseEvent & e, OIS::MouseButtonID id) { return true; }

			/**
			 * @see KeyListener::keyPressed().
			 * Can be implemented as needed.
			 */
			virtual bool keyPressed(const OIS::KeyEvent & e) { return true; }

			/**
			 * @see KeyListener::keyReleased().
			 * Can be implemented as needed.
			 */
			virtual bool keyReleased(const OIS::KeyEvent & e) { return true; }

			/**
			 * Initialise the manager for this game state
			 */
			void setManager(GameStateManager* manager);

		protected:
			/**
			 * Switch to a new gamestate
			 */
			void switchGameState(GameStateManager::GameState state);
	};
}

#endif //BASICGAMESTATE_H