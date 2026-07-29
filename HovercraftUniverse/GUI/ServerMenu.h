#ifndef _SERVERMENU_H
#define _SERVERMENU_H

#include "OverlayContainer.h"
#include "ConnectListener.h"
#include "ConnectWindow.h"
#include "MenuButton.h"
#include "ServerMenuListener.h"

namespace HovUni {
	/**
	 * An overlay containing a server menu
	 *
	 * @author Nick De Frangh
	 */
	class ServerMenu : public OverlayContainer {
		private:
			/** The background overlay */
			BasicOverlay* mBackground;

			/** The join button overlay */
			MenuButton* mJoinButton;

			/** The create button overlay */
			MenuButton* mCreateButton;

			/** The back button overlay */
			MenuButton* mBackButton;

			/** The connect window overlay */
			ConnectWindow* mConnectWindow;

			/** The listener who wants our callbacks */
			ServerMenuListener* mListener;

			/** A Listener that will listen for connection results */
			ConnectListener *mConnectListener;

		public:
			/**
			 * Basic constructor.
			 *
			 * @param listener The listener who wants callbacks from the user
			 * @param onBack A function to call when the back button has been pressed
			 */
			ServerMenu(ServerMenuListener* listener, const Hikari::FlashDelegate& onBack, ConnectListener* connectListener);

			/**
			 * Destructor
			 */
			virtual ~ServerMenu();

			/**
			 * Function that will be called when a user presses the ok button
			 *
			 * @param caller The control that called this function (don't use unless you know how to!)
			 * @param args The arguments that were passed to this function (0: String (IP))
			 * @return A return value for the flash movie
			 */
			Hikari::FlashValue onOk(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			/**
			 * Function that will be called when a user presses the cancel button
			 *
			 * @param caller The control that called this function (don't use unless you know how to!)
			 * @param args The arguments that were passed to this function (empty)
			 * @return A return value for the flash movie
			 */
			Hikari::FlashValue onCancel(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			/**
			 * Function that will be called when a user presses the create button
			 *
			 * @param caller The control that called this function (don't use unless you know how to!)
			 * @param args The arguments that were passed to this function (empty)
			 * @return A return value for the flash movie
			 */
			Hikari::FlashValue onCreate(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			/**
			 * Function that will be called when a user presses the join button
			 *
			 * @param caller The control that called this function (don't use unless you know how to!)
			 * @param args The arguments that were passed to this function (empty)
			 * @return A return value for the flash movie
			 */
			Hikari::FlashValue onJoin(Hikari::FlashControl* caller, const Hikari::Arguments& args);
	};
}

#endif //_SERVERMENU_H