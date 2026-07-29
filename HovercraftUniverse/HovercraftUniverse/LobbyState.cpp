#include "LobbyState.h"

#include "Config.h"
#include "Application.h"
#include "InGameState.h"
#include "EntityMapping.h"
#include <tinyxml/tinyxml.h>
#include <OgreRoot.h>

namespace HovUni {
	LobbyState::LobbyState(HUClient* client) : mClient(client), mLobby(client->getLobby()), mLastGUIUpdate(0), mLastClientUpdate(0) {
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
				mLobbyGUI->addUser(player->getID(), player->getPlayerName(), player->getCharacter(), player->getHovercraft());
			}
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
