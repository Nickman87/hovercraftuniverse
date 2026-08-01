#include "LobbyState.h"

#include "Config.h"
#include "Application.h"
#include "InGameState.h"
#include "EntityMapping.h"
#include <tinyxml/tinyxml.h>
#include <OgreRoot.h>

namespace HovUni {
	LobbyState::LobbyState(HUClient* client) : mClient(client), mLobby(client->getLobby()), mLastGUIUpdate(0), mLastClientUpdate(0), mAutoStartTriggered(false), mAutoStartWaitLogged(false) {
		mGUIManager = GUIManager::getSingletonPtr();
		mLobbyGUI = new LobbyGUI(Hikari::FlashDelegate(this, &LobbyState::hovercraftChange), Hikari::FlashDelegate(this, &LobbyState::mapChange), Hikari::FlashDelegate(this, &LobbyState::onChat), Hikari::FlashDelegate(this, &LobbyState::onPressStart), Hikari::FlashDelegate(this, &LobbyState::onPressLeave), Hikari::FlashDelegate(this, &LobbyState::botsValue), Hikari::FlashDelegate(this, &LobbyState::playerMax));
	}

	LobbyState::~LobbyState() {
		delete mLobbyGUI;

		for (map<int, PlayerSettingsInterceptor*>::iterator it = mPlayerInterceptors.begin(); it != mPlayerInterceptors.end(); ++it) {
			//Delete the interceptor
			delete (*it).second;
		}
	}

	Hikari::FlashValue LobbyState::onChat(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		Ogre::String chatText = args.at(0).getString();

		if (chatText != "") {
			mClient->getChat()->sendText(chatText);
			Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "[LobbyState]: " << "Sending chat message: " << args.at(0).getString();
		}

		return "success";
	}

	Hikari::FlashValue LobbyState::onPressStart(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		mLobby->start();

		return "success";
	}

	Hikari::FlashValue LobbyState::onPressLeave(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		//Disconnect
		string username = mClient->getLobby()->getOwnPlayer()->getPlayerName();
		mClient->disconnect(username + " is leaving");

		mManager->switchState(GameStateManager::MAIN_MENU);
		//Delete the client to save some resources
		delete mClient;

		return "success";
	}

	Hikari::FlashValue LobbyState::botsValue(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		bool fillWithBots = args.at(0).getBool();

		mLobby->setBots(fillWithBots);

		return "success";
	}

	Hikari::FlashValue LobbyState::playerMax(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		int maxPlayers = (int)args.at(0).getNumber();
		
		mLobby->setMaxPlayers(maxPlayers);

		return "success";
	}

	Hikari::FlashValue LobbyState::mapChange(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		int mapID = (int)args.at(0).getNumber();

		mLobby->setTrackId(mapID);

		return "success";
	}

	Hikari::FlashValue LobbyState::hovercraftChange(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		int hoverID = (int)args.at(0).getNumber();

		mLobby->getOwnPlayer()->setHovercraft(hoverID);

		return "success";
	}

	////////////////////////////////////////
	//	PlayerSettingsListener functions
	////////////////////////////////////////

	void LobbyState::onPlayerUpdate(int id, const std::string& username, const std::string& character, const std::string& car) {
		// Log what actually reaches the lobby GUI. The C++ -> Flash push is
		// write-only (Hikari returns <undefined/> and the SWF cannot be
		// queried), so without this there is no way to tell "the GUI was
		// never told" from "the GUI was told and did not render it" -- the
		// distinction that took the longest to establish when the lobby was
		// showing blank names. Must be here, at the top: the delayed-user
		// branch below returns early, and that is precisely the branch a
		// late-arriving replicated name takes. Cheap: lobby-rate, not
		// frame-rate.
		Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream()
			<< "[LobbyState]: update user " << id << " '" << username << "' "
			<< character << "/" << car;

		//check if this user has already been announced
		if (username != "") {
			std::vector<unsigned int>::const_iterator it = mDelayedUsers.begin();
			while( it != mDelayedUsers.end() ) {
				if ( (*it) == id ) {
					//Add the user, and delete it from the vector
					mLobbyGUI->addUser(id, username, character, car);
					mDelayedUsers.erase(it);
					return;
				}
				++it;
			}
		}

		//Player has been updated, propagate changes
		mLobbyGUI->editUser(id, username, character, car);

		if (id == mLobby->getOwnPlayer()->getID()) {
			mLobbyGUI->setHovercraft(mLobby->getOwnPlayer()->getHovercraftID(), mLobby->getOwnPlayer()->getHovercraft());
		}
	}

	////////////////////////////////////////
	//	LobbyListener functions
	////////////////////////////////////////

	void LobbyState::onLeave(ZCom_ConnID id) {
		mLobbyGUI->deleteUser(id);
	}

	void LobbyState::onJoin(PlayerSettings * settings) {
		//Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "[LobbyState]: onJoin was called! " << settings->getPlayerName() << " (" << settings->getID() << ")";
		//Player has joined, add empty player to the visualisation
		Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream()
			<< "[LobbyState]: join user " << settings->getID() << " '"
			<< settings->getPlayerName() << "'"
			<< (settings->getPlayerName() == "" ? " (name not yet replicated, deferred)" : "");
		if (settings->getPlayerName() != "") {
			mLobbyGUI->addUser(settings->getID(), settings->getPlayerName(), settings->getCharacter(), settings->getHovercraft());
		} else {
			mDelayedUsers.push_back(settings->getID());
		}
		//Create a new playersettingsinterceptor
		PlayerSettingsInterceptor* intercept = new PlayerSettingsInterceptor(settings, this);
		//Store te interceptor
		mPlayerInterceptors.insert(std::pair<int, PlayerSettingsInterceptor*>(settings->getID(), intercept));
	}

	void LobbyState::onStart() {
		//We need to start loading
		TiXmlDocument doc("gui/GUIConfig.xml");
		doc.LoadFile();
		InGameState* newState = new InGameState(mClient, mLobby->getRaceState(), doc.RootElement()->FirstChildElement("HUD"));
		mManager->addGameState(GameStateManager::IN_GAME, newState);
		mManager->switchState(GameStateManager::IN_GAME);
	}

	void LobbyState::onAdminChange(bool isAdmin) {
		mLobbyGUI->showStart(isAdmin);
		mLobbyGUI->setAdmin(isAdmin);
	}
	
	void LobbyState::onBotsChange(bool fillWithBots) {
		mLobbyGUI->setFillBots(fillWithBots);
	}

	void LobbyState::onMaxPlayersChange(int players) {
		mLobbyGUI->setPlayerMax(players);
	}

	void LobbyState::onTrackChange(int trackid) {
		std::map<unsigned int, Ogre::String> maps = EntityMapping::getInstance().getMap(EntityMapping::MAPS);
		mLobbyGUI->setMap(trackid, maps[trackid]);
	}

	////////////////////////////////////////
	//	BasicGameState functions
	////////////////////////////////////////

	void LobbyState::activate() {
		//Register for chatevents
		mClient->setChatListener(mLobbyGUI);

		//Register for lobby events
		mClient->getLobby()->addListener(this);

		//We don't want any crazy input keys
		mInputManager->getKeyManager()->setInactive();

		//Make sure we have a cursor
		mGUIManager->showCursor(true);

		//Activate the menu overlay
		mLobbyGUI->activate();

		//Show ourselves in the lobby!
		//Get all the playerconfigs and show them in the lobby
		const Lobby::playermap::list_type players = mLobby->getPlayers();
		for (Lobby::playermap::const_iterator i = players.begin(); i != players.end(); ++i) {
			PlayerSettings* player = (*i).second;
			if (!player->isBot()) {
				// Logged for the same reason as onJoin()/onPlayerUpdate():
				// players already present when this state activates never
				// pass through either of those, so without this they would
				// be the one group of lobby entries with no trace at all of
				// what the GUI was told about them.
				Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream()
					<< "[LobbyState]: existing user " << player->getID() << " '"
					<< player->getPlayerName() << "'"
					<< (player->getPlayerName() == "" ? " (name not yet replicated)" : "");
				mLobbyGUI->addUser(player->getID(), player->getPlayerName(), player->getCharacter(), player->getHovercraft());
			}

			// onJoin() (which normally creates this
			// player's PlayerSettingsInterceptor) is dispatched by
			// Lobby::addPlayer() as soon as the dynamic PlayerSettings node
			// is created -- for our own node, that happens on HUClient's
			// background connect thread (ClientConnectThread), synchronously
			// within its very first ZCom process() call, since the shim's
			// in-process ENet transport drains the whole connect handshake
			// (connect reply + Lobby/PlayerSettings NODE_CREATE + the
			// InitEvent round trip) in one go -- well before this activate()
			// call ever runs on the main thread and registers as a Lobby
			// listener (mClient->getLobby()->addListener(this), above). Any
			// player already present at this point (in practice: our own
			// entry) therefore never reached LobbyState::onJoin() and has no
			// interceptor, so PlayerSettingsInterceptor::in/outPostUpdate()
			// never has anything to call onPlayerUpdate() on -- silently
			// dropping the one-time initial sync (editUser/setHovercraft)
			// that onPlayerUpdate() would otherwise have delivered. Create
			// the missing interceptor here, exactly like onJoin() does, for
			// any player this instance hasn't already seen.
			if (mPlayerInterceptors.find(player->getID()) == mPlayerInterceptors.end()) {
				PlayerSettingsInterceptor* intercept = new PlayerSettingsInterceptor(player, this);
				mPlayerInterceptors.insert(std::pair<int, PlayerSettingsInterceptor*>(player->getID(), intercept));
			}
		}

		// Same reasoning as above: if onPlayerUpdate() was never dispatched
		// for our own player, the hovercraft selection box was never told
		// what we picked (only onPlayerUpdate() calls setHovercraft() for
		// the local player -- see onPlayerUpdate() above). Make sure the
		// selection box reflects our own already-configured choice now.
		if (mLobby->getOwnPlayer()) {
			mLobbyGUI->setHovercraft(mLobby->getOwnPlayer()->getHovercraftID(), mLobby->getOwnPlayer()->getHovercraft());
		}

		//Activate all possible interception listeners
		for (map<int, PlayerSettingsInterceptor*>::iterator it = mPlayerInterceptors.begin(); it != mPlayerInterceptors.end(); ++it) {
			//make the interceptor inactive
			(*it).second->setStatus(true);
		}
		
		//Set some initial gui values
		onAdminChange(mLobby->isAdmin());
		onBotsChange(mLobby->hasBots());
		onMaxPlayersChange(mLobby->getMaxPlayers());

		//Store the maps
		std::map<unsigned int, Ogre::String> maps = EntityMapping::getInstance().getMap(EntityMapping::MAPS);
		std::map<unsigned int, Ogre::String>::iterator it;
		mLobbyGUI->clearMaps();
		for(it = maps.begin(); it != maps.end(); it++) {
			mLobbyGUI->addMap(it->first, it->second);
		}
		onTrackChange(mLobby->getTrackId());

		//Store the hovercrafts
		std::map<unsigned int, Ogre::String> hovercrafts = EntityMapping::getInstance().getMap(EntityMapping::HOVERCRAFT);
		mLobbyGUI->clearHovercrafts();
		for(it = hovercrafts.begin(); it != hovercrafts.end(); it++) {
			mLobbyGUI->addHovercraft(it->first, it->second);
		}
	}

	void LobbyState::disable() {
		//Remove us from the chat events
		mClient->removeChatListener(mLobbyGUI);

		//Disable all the interception listeners
		for (map<int, PlayerSettingsInterceptor*>::iterator it = mPlayerInterceptors.begin(); it != mPlayerInterceptors.end(); ++it) {
			//make the interceptor inactive
			(*it).second->setStatus(false);
		}

		//Deactivate the menu overlay
		mLobbyGUI->deactivate();

		//Deregister for lobby events
		mClient->getLobby()->removeListener(this);
	}

	bool LobbyState::frameStarted(const Ogre::FrameEvent & evt) {
		bool result = true;

		// Test affordance (revival Phase B, docs/porting/phase-b-plan.md):
		// --autostart lets a two-process test harness start the race itself
		// once it reaches the lobby, with no GUI interaction. Admin status
		// arrives over the network (it's granted by the server once our
		// PlayerSettings/Lobby node has linked, see Lobby.cpp), so this polls
		// isAdmin() every tick rather than giving up after the first one.
		// Reuses the exact same mLobby->start() call as onPressStart() (the
		// "Start" button) above. Without --autostart,
		// Application::getAutoStart() is false and this whole block is a
		// no-op: behaviour is unchanged from before this affordance existed.
		if (!mAutoStartTriggered && Application::getAutoStart()) {
			if (mLobby->isAdmin()) {
				mAutoStartTriggered = true;

				Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream()
					<< "[Autostart]: starting race as admin";

				mLobby->start();
			} else if (!mAutoStartWaitLogged) {
				mAutoStartWaitLogged = true;

				Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream()
					<< "[Autostart]: waiting on admin status before starting the race";
			}
		}

		mLastGUIUpdate += evt.timeSinceLastFrame;
		mLastClientUpdate += evt.timeSinceLastFrame;

		//50 FPS
		if (mLastGUIUpdate > (1.0f / 50.0f) || mLastGUIUpdate < 0) {
			if (mLobbyGUI->isActivated()) {
				mLobbyGUI->markAdmin(mLobby->getAdminId());
			}
			
			//We are using a GUI, so update it
			mGUIManager->update();
			mLastGUIUpdate = 0.0f; //Reset
		}

		if (mLastClientUpdate > 0.016f) {
			mClient->process((int) (mLastClientUpdate * 1000.0f));
			mLastClientUpdate = 0.0f;
		}

		//We have sound, update it
		mSoundManager->update();

		return result;
	}

	bool LobbyState::mouseMoved(const OIS::MouseEvent & e) {
		bool result = true;

		//We are using a GUI, so update it
		result = result && mGUIManager->mouseMoved(e);

		return result;
	}

	bool LobbyState::mousePressed(const OIS::MouseEvent & e, OIS::MouseButtonID id) {
		bool result = true;

		//We are using a GUI, so update it
		result = result && mGUIManager->mousePressed(e, id);
		
		return result;
	}

	bool LobbyState::mouseReleased(const OIS::MouseEvent & e, OIS::MouseButtonID id) {
		bool result = true;

		//We are using a GUI, so update it
		result = result && mGUIManager->mouseReleased(e, id);
		
		return result;
	}

	bool LobbyState::keyPressed(const OIS::KeyEvent & e) {
		bool result = true;

		//We are using a GUI, so update it
		result = result && mGUIManager->keyPressed(e);
		
		return result;
	}

	bool LobbyState::keyReleased(const OIS::KeyEvent & e) {
		bool result = true;

		//We are using a GUI, so update it
		result = result && mGUIManager->keyReleased(e);
		
		return result;
	}
}
