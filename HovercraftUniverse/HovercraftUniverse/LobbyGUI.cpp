#include "LobbyGUI.h"
#include "GUIManager.h"
#include "StringUtils.h"

namespace HovUni {
	LobbyGUI::LobbyGUI(const Hikari::FlashDelegate& hovercraftChange, const Hikari::FlashDelegate& mapChange, const Hikari::FlashDelegate& onChat, const Hikari::FlashDelegate& onStart, const Hikari::FlashDelegate& onLeave, const Hikari::FlashDelegate& botsValue, const Hikari::FlashDelegate& playerMax) {
		//Show the background
		mBackground = new BasicOverlay("Background_LB", "background.swf", GUIManager::getSingletonPtr()->getResolutionWidth(), GUIManager::getSingletonPtr()->getResolutionHeight(), Hikari::Center, 1);
		mBackground->setExactFit(true);
		addOverlay("Background__LB", mBackground);
		
		//Lets make the buttons first, calculate their size
		int height = GUIManager::getSingletonPtr()->getResolutionHeight() / 5; //We want to fill the entire screen
		float scale = (height * 1.0f) / 320.0f;
		int width = (int) (445 * scale);
		
		//mPickCar = new MenuButton("Change\nHovercraft", "pickCarBtn", "menuButton.swf", width, height, Hikari::Position(Hikari::Right, 0, -(3 * (height / 2))));
		//mSingleplayerButton->bind("press", Hikari::FlashDelegate(this, &MainMenu::onSingleplayer));;
		//mPickChar = new MenuButton("Change\nCharacter", "pickCharBtn", "menuButton.swf", width, height, Hikari::Position(Hikari::Right, 0, -(height / 2)));
		mLeave = new MenuButton("Leave Lobby", "leaveBtn", "menuButton.swf", width, height, Hikari::Position(Hikari::Right, 0, (2 * (height / 2))));
		mLeave->bind("press", onLeave);
		mStart = new MenuButton("Start", "startBtn", "menuButton.swf", width, height, Hikari::Position(Hikari::Right, 0, (4 * (height / 2))));
		mStart->bind("press", onStart);

		//addOverlay("pickCarBtn", mPickCar);
		//addOverlay("pickCharBtn", mPickChar);
		addOverlay("leaveBtn", mLeave);

		int lobbyWidth = GUIManager::getSingletonPtr()->getResolutionWidth() - width; //The maximum width we have
		scale = (lobbyWidth * 1.0f) / 1500.0f;
		int lobbyHeight = (int) (scale * 1000);

		//Now show the lobby itself
		mLobbyOverlay = new LobbyOverlay(hovercraftChange, mapChange, onChat, botsValue, playerMax, "lobbyVisual", "lobby.swf", lobbyWidth, lobbyHeight, Hikari::BottomLeft);
		addOverlay("lobbyVisual", mLobbyOverlay);
	}

	LobbyGUI::~LobbyGUI() {
		if (mStart->isActivated()) {
			GUIManager::getSingletonPtr()->disableOverlay(mStart);
		}

		//delete mPickCar;
		//delete mPickChar;
		delete mLeave;
		delete mStart;
		delete mLobbyOverlay;
	}

	void LobbyGUI::onDeactivate() {
		if (mStart->isActivated()) {
			GUIManager::getSingletonPtr()->disableOverlay(mStart);
		}
	}

	std::string LobbyGUI::stripString(const std::string& value) {
		return StringUtils::htmlSpecialChars(value);
	}

	void LobbyGUI::newMessage(const std::string& user, const std::string& line) {
		mLobbyOverlay->addText( stripString(user), stripString(line) );
	}

	void LobbyGUI::newNotification(const std::string& notif) {
		mLobbyOverlay->addAction( stripString(notif) );
	}

	void LobbyGUI::addUser(int id, const std::string& username, const std::string& character, const std::string& car) {
		mLobbyOverlay->addUser(id, username, character, car);
	}

	void LobbyGUI::editUser(int id, const std::string& username, const std::string& character, const std::string& car) {
		mLobbyOverlay->editUser(id, username, character, car);
	}

	void LobbyGUI::deleteUser(int id) {
		mLobbyOverlay->deleteUser(id);
	}

	void LobbyGUI::setTrackId(unsigned int value){
		
	}

	void LobbyGUI::setAdmin(bool admin) {
		mLobbyOverlay->setAdmin(admin);
	}

	void LobbyGUI::markAdmin(int id) {
		mLobbyOverlay->markAdmin(id);
	}

	void LobbyGUI::setFillBots(bool fill) {
		mLobbyOverlay->setFillBots(fill);
	}

	void LobbyGUI::setPlayerMax(int value) {
		mLobbyOverlay->setPlayerMax(value);
	}

	void LobbyGUI::showStart(bool show) {
		if (show) {
			//Make sure the button is activated
			if (!mStart->isActivated()) {
				GUIManager::getSingletonPtr()->activateOverlay(mStart);
			}
		} else {
			//Make sure the button is deactivated
			if (mStart->isActivated()) {
				GUIManager::getSingletonPtr()->disableOverlay(mStart);
			}
		}
	}

	void LobbyGUI::clearMaps() {
		mLobbyOverlay->clearMaps();
	}
			
	void LobbyGUI::addMap(int id, const std::string& name) {
		mLobbyOverlay->addMap(id, name);
	}

	void LobbyGUI::setMap(int id, const std::string& name) {
		mLobbyOverlay->setMap(id, name);
	}

	void LobbyGUI::clearHovercrafts() {
		mLobbyOverlay->clearHovercrafts();
	}

	void LobbyGUI::addHovercraft(int id, const std::string& name) {
		mLobbyOverlay->addHovercraft(id, name);
	}

	void LobbyGUI::setHovercraft(int id, const std::string& name) {
		mLobbyOverlay->setHovercraft(id, name);
	}
}