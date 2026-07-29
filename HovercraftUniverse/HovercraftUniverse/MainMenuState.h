#ifndef MAINMENUSTATE_H
#define MAINMENUSTATE_H

#include "BasicGameState.h"

#include "ClientConnectThread.h"
#include "ConnectListener.h"
#include "MainMenu.h"
#include "ServerMenuListener.h"
#include "HUDedicatedServer.h"

namespace HovUni {
	/**
	 * A Game state representing the main menu
	 *
	 * @author Nick De Frangh
	 */
	class MainMenuState : public BasicGameState, public ServerMenuListener, public ConnectListener {
		private:
			/** The Overlay for the Main menu */
			MainMenu* mMenu;

			/** Time since last GUI update */
			Ogre::Real mLastGUIUpdate;

			/** Boolean to check if we want to stop or not */
			bool mContinue;
	
			/** A reference to the connection thread that was used */
			ClientConnectThread* mConnectionThread;

			/** A connect listener if someone was listening for connections */
			ConnectListener* mConnectListener;

			/** Bool to check if a connection has finished */
			bool mConnectionFinished;

			/** the result of the connection attempt */
			bool mConnectionResult;

			/** The local server, for single player (Dirk)*/
			HUDedicatedServer* mLocalServer;

		public:
			/**
			 * Constructor for the Main menu state
			 */
			MainMenuState();

			/**
			 * Destructor
			 */
			~MainMenuState();

			/**
			 * this function gets calles when we want to connect to a certain IP
			 *
			 * @param address The adress to connect to <IP:port> or <IP>
			 * @param listener The listener to notify the end of the connection to
			 */
			virtual void onConnect(const Ogre::String& address, ConnectListener* listener);

			/**
			 * This function will be called when the client connection thread has finished
			 *
			 * @param success True when the connection was successfull, false otherwise
			 */
			virtual void onConnectFinish(bool success);

			/**
			 * Called when we need to create a multiplayer game
			 */
			virtual void onCreate();

			/**
			 * Function that has to be called when the connection was succesfull.
			 */
			virtual void finishConnect();

			/**
			 * Function that will be called when we want to quit the game
			 *
			 * @param caller A Reference to the caller, should be ignored
			 * @param arg Some arguments, will be empty
			 */
			Hikari::FlashValue onQuit(Hikari::FlashControl* caller, const Hikari::Arguments& args);

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
	};
}

#endif //MAINMENUSTATE_H