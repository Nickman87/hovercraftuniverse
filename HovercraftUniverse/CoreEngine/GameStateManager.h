#ifndef GAMESTATEMANAGER_H
#define GAMESTATEMANAGER_H

#include <map>

#include "InputManager.h"
#include <Ogre.h>
#include <OgreFrameListener.h>

namespace HovUni {
	class BasicGameState; //Forward declaration

	/**
	 * This class manages the different gamestates.
	 * For example: MainMenu, SinglePlayer, Multiplayern, ...
	 *
	 * @author Nick De Frangh
	 */
	class GameStateManager : public Ogre::FrameListener, public OIS::MouseListener, public OIS::KeyListener {
		public:
			/** Enumeration for all possible gamestates */
			static enum GameState {
				MAIN_MENU = 0, LOBBY, IN_GAME
			};

		private:
			/** A map containing all the regitered game states */
			std::map<GameState, BasicGameState*> mGameStates;

			/** A reference to the input manager */
			InputManager* mInputManager;

			/** The current state of the game */
			GameState mCurrentState;

			/** A reference to the active game state */
			BasicGameState* mCurrentGameState;

		public:
			/** 
			 * Basic constructor
			 */
			GameStateManager(InputManager * inputMgr, GameState initialState, BasicGameState* gameState);

			/**
			 * Add a new gamestate to the manager. Old states CAN be replaced! This
			 * will call delete on the pointer!
			 *
			 * @param state The identifier for this type of state
			 * @param gameState The implementation for this game state
			 */
			void addGameState(GameState state, BasicGameState* gameState);

			/**
			 * Switch to a new state, can only be accessed by friend
			 * classes (the game states).
			 *
			 * @param state The new stat to switch to
			 */
			void switchState(GameState state);

			/**
			 * @see FrameListener::frameStarted().
			 */
			bool frameStarted(const Ogre::FrameEvent & evt);

			/**
			 * @see MouseListener::mouseMoved().
			 */
			bool mouseMoved(const OIS::MouseEvent & e);

			/**
			 * @see MouseListener::mousePressed().
			 */
			bool mousePressed(const OIS::MouseEvent & e, OIS::MouseButtonID id);
			
			/**
			 * @see MouseListener::mouseReleased().
			 */
			bool mouseReleased(const OIS::MouseEvent & e, OIS::MouseButtonID id);

			/**
			 * @see KeyListener::keyPressed().
			 */
			bool keyPressed(const OIS::KeyEvent & e);

			/**
			 * @see KeyListener::keyReleased().
			 */
			bool keyReleased(const OIS::KeyEvent & e);
	};
}

#endif //GAMESTATEMANAGER_H