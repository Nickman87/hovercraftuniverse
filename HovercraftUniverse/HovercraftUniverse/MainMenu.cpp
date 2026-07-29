#include "MainMenu.h"
#include <NMessageBox.h>
#include "GUIManager.h"
#include <windows.h>

namespace HovUni {
	MainMenu::MainMenu(ServerMenuListener* serverListener, const Hikari::FlashDelegate& onQuit) {
		mLocalServer = 0;
		//Create buttons
		mListener = serverListener;

		//Show the background
		mBackground = new BasicOverlay("Background_MM", "background.swf", GUIManager::getSingletonPtr()->getResolutionWidth(), GUIManager::getSingletonPtr()->getResolutionHeight(), Hikari::Center, 1);
		mBackground->setExactFit(true);
		addOverlay("Background__MM", mBackground);

		std::pair<int, int> size = GUIManager::getSingletonPtr()->scale(222, 160, 445, 320);
		int width = size.first;
		int height = size.second;

		mSingleplayerButton = new MenuButton("Singleplayer", "singleplayerBtn", "menuButton.swf", width, height, Hikari::Position(Hikari::Center, -(width / 2), -(height / 2)));
		mSingleplayerButton->bind("press", Hikari::FlashDelegate(this, &MainMenu::onSingleplayer));
		mMultiplayerButton = new MenuButton("Multiplayer", "multiplayerBtn", "menuButton.swf", width, height, Hikari::Position(Hikari::Center, (width / 2), -(height / 2)));
		mMultiplayerButton->bind("press", Hikari::FlashDelegate(this, &MainMenu::onMultiplayer));
		//mOptionsButton = new MenuButton("Options", "optionsBtn", "menuButton.swf", width, height, Hikari::Position(Hikari::Center, -(width / 2), (height / 2)));
		//mOptionsButton->bind("press", Hikari::FlashDelegate(this, &MainMenu::onOptions));
		mQuitButton = new MenuButton("Quit", "quitBtn", "menuButton.swf", width, height, Hikari::Position(Hikari::Center, 0, (height / 2)));
		mQuitButton->bind("press", onQuit);

		addOverlay("singleplayerBtn", mSingleplayerButton);
		addOverlay("multiplayerBtn", mMultiplayerButton);
		//addOverlay("optionsBtn", mOptionsButton);
		addOverlay("quitBtn", mQuitButton);

		//Calculate inition top padding, this is the total space available above the buttons
		int topPadding = ((GUIManager::getSingletonPtr()->getResolutionHeight() / 2) - height);
		size = GUIManager::getSingletonPtr()->scale(356, 86, 713, 173);
		width = size.first;
		height = size.second;
		
		//Adjust the padding to the calculated height
		//topPadding -= (height - (height / 3));
		topPadding = (topPadding / 2) - (height / 4);

		mTitle = new BasicOverlay("Title", "title.swf", width, height, Hikari::Position(Hikari::TopCenter, 0, topPadding));
		mTitle->setBParameter(BasicOverlay::ALPHAHACK, true);
		addOverlay("title", mTitle);

		mServerMenu = new ServerMenu(serverListener, Hikari::FlashDelegate(this, &MainMenu::onBack), this);
	}

	MainMenu::~MainMenu() {
		if (isActivated()) {
			deactivate();
		}
		if (mServerMenu->isActivated()) {
			mServerMenu->deactivate();
		}
		//Delete / stop the server if it is running
		if (mLocalServer != 0) {
			mLocalServer->stop();
			delete mLocalServer;
			mLocalServer = 0;
		}

		delete mSingleplayerButton;
		delete mMultiplayerButton;
		//delete mOptionsButton;
		delete mQuitButton;
		delete mTitle;
		delete mServerMenu;
		delete mBackground;
	}

	void MainMenu::onActivate() {
	}

	void MainMenu::onDeactivate() {
		if (mServerMenu->isActivated()) {
			mServerMenu->deactivate();
		}
	}

	Hikari::FlashValue MainMenu::onSingleplayer(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		//mListener->onConnect("localhost", this);
		//Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "[MainMenu]: onsingleplayer finished";
		
		//NEW CODE THAT STARTS THE SERVER::
		try {
			//HovUni::Console::createConsole("HovercraftUniverse Dedicated Server");
			mLocalServer = new HUDedicatedServer("SingleplayerServer.ini");
		
			mLocalServer->init();
			mLocalServer->run(false);

			//Give the server a second to load, should be enough
			DWORD dwMilliseconds = 1000;
			Sleep(dwMilliseconds);

			//Todo prettify catch blocks error msgs like this:
			//HovUni::MessageBox* msg = new MessageBox("Could not connect to server", "connectionmessage");
			//GUIManager::getSingletonPtr()->activateOverlay(msg);
		
			//HovUni::Console::destroyConsole();
			mListener->onConnect("localhost", this);
		
			Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "[MainMenu]: onsingleplayer finished";

		} catch (Ogre::Exception& e) {
			GUIManager::getSingletonPtr()->activateOverlay(new HovUni::NMessageBox(e.getFullDescription(), "OgreException"));
			//MessageBoxA(NULL, e.getFullDescription().c_str(), "Ogre Exception in starting Single Player Server", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		} catch (HovUni::Exception& e2) {
			GUIManager::getSingletonPtr()->activateOverlay(new HovUni::NMessageBox(e2.getMessage(), "HovUniException"));
			//MessageBoxA(NULL, e2.getMessage().c_str(), "HovUni Exception in starting Single Player Server", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		} catch (std::exception& e) {
			GUIManager::getSingletonPtr()->activateOverlay(new HovUni::NMessageBox(e.what(), "stdException"));
			//MessageBoxA(NULL, e.what(), "Exception in starting Single Player Server", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		} catch (...) {
			GUIManager::getSingletonPtr()->activateOverlay(new HovUni::NMessageBox("Unknown fatal Ogre Exception in starting Single Player Server", "unknownException"));
			//MessageBoxA(NULL, "Unknown fatal Ogre Exception in starting Single Player Server", "Exception in starting Single Player Server", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		}
		

		return "success";
	}

	Hikari::FlashValue MainMenu::onMultiplayer(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		//Deactivate ourself
		this->deactivate();

		//Activate the ServerMenu
		mServerMenu->activate();

		return "success";
	}

	Hikari::FlashValue MainMenu::onOptions(Hikari::FlashControl* caller, const Hikari::Arguments& args) {

		return "success";
	}

	Hikari::FlashValue MainMenu::onBack(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		//We came back to the main menu
		mServerMenu->deactivate();

		//activate ourself
		this->activate();

		return "success";
	}

	void MainMenu::onConnectFinish(bool success) {
		if (success) {
			//Deactivate ourselves
			mServerMenu->deactivate();

			//Notify the listener to finish the connection
			mListener->finishConnect();
		} else {
			//Show a messagebox
			GUIManager::getSingletonPtr()->activateOverlay(new HovUni::NMessageBox("Could not connect to server", "connectionmessage"));
		}
	}
}