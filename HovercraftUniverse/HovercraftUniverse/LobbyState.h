#ifndef _LOBBYSTATE_H
#define _LOBBYSTATE_H

#include <BasicGameState.h>
#include "HUClient.h"
#include "PlayerSettingsListener.h"
#include "PlayerSettingsInterceptor.h"
#include "Lobby.h"
#include "LobbyGUI.h"
#include "LobbyListener.h"
#include <map>
#include <vector>

namespace HovUni {
	/**
	 * This class represents the lobby state of the game. When a client
	 * connects to a server and a game has not started, the client will
	 * reside in this state.
	 *
	 * @author Nick De Frangh
	 */
	class LobbyState : public BasicGameState, public LobbyListener, public PlayerSettingsListener {
		private:
			/** The client which represents our connection */
			HUClient* mClient;

			/** The lobby we are in*/
			Lobby* mLobby;

			/** A reference to the GUI manager */
			GUIManager* mGUIManager;

			/** The vieualisation for the GUI */
			LobbyGUI* mLobbyGUI;

			/** Time since last GUI update, in seconds */
			Ogre::Real mLastGUIUpdate;

			/** Time since the last client update, in seconds */
			Ogre::Real mLastClientUpdate;

			/** Mapping of ID's to playersettingsinterceptors */
			std::map<int, PlayerSettingsInterceptor*> mPlayerInterceptors;

			/** The ID's of users that have not yet been registered with the gui */
			std::vector<unsigned int> mDelayedUsers;

		public:
			/**
			 * Constructor
			 *
			 * @param client The client to which the user is connected
			 */
			LobbyState(HUClient* client);

			/** Destructor */
			virtual ~LobbyState();

			/**
			 * Function that will handle new Chat Input
			 *
			 * @param caller A Reference to the caller, should be ignored unles syou know what you are doing!
			 * @param arg The arguments of the called function (string)
			 * @return Should return the return value that is axcpected by the flash file or something random
			 *		if void.
			 */
			Hikari::FlashValue onChat(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			/**
			 * Function that will be called when a user clicks on the start button
			 *
			 * @param caller A Reference to the caller, should be ignored unles syou know what you are doing!
			 * @param arg The arguments of the called function (empty)
			 * @return Should return the return value that is axcpected by the flash file or something random
			 *		if void.
			 */
			Hikari::FlashValue onPressStart(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			/**
			 * Function that will be called when a user clicks on the leave button
			 *
			 * @param caller A Reference to the caller, should be ignored unles syou know what you are doing!
			 * @param arg The arguments of the called function (empty)
			 * @return Should return the return value that is axcpected by the flash file or something random
			 *		if void.
			 */
			Hikari::FlashValue onPressLeave(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			/**
			 * Function that will be called when the "fill with bots" value of the lobby is changed by the admin.
			 *
			 * @param caller A Reference to the caller, should be ignored unles syou know what you are doing!
			 * @param arg The arguments of the called function (bool)
			 * @return Should return the return value that is axcpected by the flash file or something random
			 *		if void.
			 */
			Hikari::FlashValue botsValue(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			/**
			 * Function that will be called when the "maximum number of players" value of the lobby is changed by the admin.
			 *
			 * @param caller A Reference to the caller, should be ignored unles syou know what you are doing!
			 * @param arg The arguments of the called function (int)
			 * @return Should return the return value that is axcpected by the flash file or something random
			 *		if void.
			 */
			Hikari::FlashValue playerMax(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			/**
			 * Function that will be called when the track is changed.
			 *
			 * @param caller A Reference to the caller, should be ignored unles syou know what you are doing!
			 * @param arg The arguments of the called function (int)
			 * @return Should return the return value that is expected by the flash file or something random
			 *		if void.
			 */
			Hikari::FlashValue mapChange(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			/**
			 * Function that will be called when the hovercraft is changed.
			 *
			 * @param caller A Reference to the caller, should be ignored unles syou know what you are doing!
			 * @param arg The arguments of the called function (int)
			 * @return Should return the return value that is expected by the flash file or something random
			 *		if void.
			 */
			Hikari::FlashValue hovercraftChange(Hikari::FlashControl* caller, const Hikari::Arguments& args);

			////////////////////////////////////////
			//	PlayerSettingsListener functions
			////////////////////////////////////////

			/**
			 * @inheritDoc
			 */
			void onPlayerUpdate(int id, const std::string& username, const std::string& character, const std::string& car);

			////////////////////////////////////////
			//	LobbyListener functions
			////////////////////////////////////////

			/**
			 * @inheritDoc
			 */
			virtual void onLeave(ZCom_ConnID id);

			/**
			 *
			 */
			virtual void onJoin(PlayerSettings * settings);

			/**
			 * @inheritDoc
			 */
			virtual void onStart();

			/**
			 * Called when the admin state of the lobby has changed
			 *
			 * @param isAdmin True when we are admin, false otherwise
			 */
			virtual void onAdminChange(bool isAdmin);

			/**
			 * Called when the fill with bots state has changed.
			 *
			 * @param fillWithBots True when this setting was enabled
			 */
			virtual void onBotsChange(bool fillWithBots);

			/**
			 * Called when the maximum number of allowed players has changed.
			 *
			 * @param players The new maximum number of players
			 */
			virtual void onMaxPlayersChange(int players);

			/**
			 * Called when the track has changed.
			 *
			 * @param trackid The new track id
			 */
			virtual void onTrackChange(int trackid);

			////////////////////////////////////////
			//	BasicGameState functions
			////////////////////////////////////////

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

#endif //_LOBBYSTATE_H
