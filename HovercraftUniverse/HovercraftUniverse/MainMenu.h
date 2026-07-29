#ifndef _MAINMENU_H
#define _MAINMENU_H

#include "ConnectListener.h"
#include "MenuButton.h"
#include "OverlayContainer.h"
#include "ServerMenu.h"
#include "ServerMenuListener.h"
#include "HUDedicatedServer.h"


namespace HovUni {
	/**
	 * An overlay that represents the main menu in the game
	 *
	 * @author Nick De Frangh
	 */
	class MainMenu : public OverlayContainer, public ConnectListener {
		private:
			/** The background overlay */
			BasicOverlay* mBackground;

			/** The singleplayer button overlay */
			MenuButton* mSingleplayerButton;

			/** The multiplayer button overlay */
			MenuButton* mMultiplayerButton;

			/** The options button overlay */
			MenuButton* mOptionsButton;

			/** The quit button overlay */
			MenuButton* mQuitButton;

			/** the title Overlay */
			BasicOverlay* mTitle;

			/** A pointer to the servermenu screen */
			ServerMenu* mServerMenu;
			
			/** A listener for input commands */
			ServerMenuListener* mListener;

			/** The local server, for single player (Dirk)*/
			HUDedicatedServer* mLocalServer;

		public:
			/**
			 * Main menu constructor
			 *
			 * @param serverListener A listener for user input (join/create)
			 * @param onQuit Function to be called when a user presses the quit button
			 */
			MainMenu(ServerMenuListener* serverListener, const Hikari::FlashDelegate& onQuit);

			/**
			 * Destructor
			 */
			virtual ~MainMenu();

			/**
			 * Function that is called on activate (empty)
			 */
			virtual void onActivate();

			/**
			 * Function that is calles on deactivate.
			 * Will make sure that everything is deactivated.
			 */
			virtual void onDeactivate();

			/**
			 * Function that will be called when a user presses the singleplayer button
			 *
			 * @param caller The control that called this function (don't use unless you know how to!)
			 * @param args The arguments that were passed to this function (empty)
			 * @return A return value for the flash movie
			 */
			Hikari::FlashValue onSingleplayer(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			/**
			 * Function that will be called when a user presses the multiplayer button
			 *
			 * @param caller The control that called this function (don't use unless you know how to!)
			 * @param args The arguments that were passed to this function (empty)
			 * @return A return value for the flash movie
			 */
			Hikari::FlashValue onMultiplayer(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			/**
			 * Function that will be called when a user presses the options button
			 *
			 * @param caller The control that called this function (don't use unless you know how to!)
			 * @param args The arguments that were passed to this function (empty)
			 * @return A return value for the flash movie
			 */
			Hikari::FlashValue onOptions(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			/**
			 * Function that will be called when a user presses the back button
			 *
			 * @param caller The control that called this function (don't use unless you know how to!)
			 * @param args The arguments that were passed to this function (empty)
			 * @return A return value for the flash movie
			 */
			Hikari::FlashValue onBack(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			/**
			 * This function will be called when connecting to the server is finished.
			 *
			 * @param success Will be true when we have a succesfull connection, false otherwise.
			 */
			void onConnectFinish(bool success);
	};
}

#endif //_MAINMENU_H