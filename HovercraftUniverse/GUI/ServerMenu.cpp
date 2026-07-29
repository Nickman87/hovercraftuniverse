#include "ServerMenu.h"
#include "GUIManager.h"

namespace HovUni {
	ServerMenu::ServerMenu(ServerMenuListener* listener, const Hikari::FlashDelegate& onBack, ConnectListener* connectListener) 
			: mListener(listener), mConnectListener(connectListener) {
		//Show the background
		mBackground = new BasicOverlay("Background_SM", "background.swf", GUIManager::getSingletonPtr()->getResolutionWidth(), GUIManager::getSingletonPtr()->getResolutionHeight(), Hikari::Center, 1);
		mBackground->setExactFit(true);
		addOverlay("Background__SM", mBackground);
		
		//Create the buttons
		std::pair<int, int> size = GUIManager::getSingletonPtr()->scale(222, 160, 445, 320);
		int width = size.first;
		int height = size.second;

		mCreateButton = new MenuButton("Create game", "createBtn", "menuButton.swf", width, height, Hikari::Position(Hikari::Center, -(width / 2), -(height / 2)));
		mCreateButton->bind("press", Hikari::FlashDelegate(this, &ServerMenu::onCreate));
		mJoinButton = new MenuButton("Join game", "joinBtn", "menuButton.swf", width, height, Hikari::Position(Hikari::Center, (width / 2), -(height / 2)));
		mJoinButton->bind("press", Hikari::FlashDelegate(this, &ServerMenu::onJoin));
		mBackButton = new MenuButton("Back", "backBtn", "menuButton.swf", width, height, Hikari::Position(Hikari::Center, 0, (height / 2)));
		mBackButton->bind("press", onBack);
		
		//Add the buttons to the overlay container
		addOverlay("createBtn", mCreateButton);
		addOverlay("joinBtn", mJoinButton);
		addOverlay("backBtn", mBackButton);

		//Create the connect window
		size = GUIManager::getSingletonPtr()->scale(341, 91, 682, 181);
		width = size.first;
		height = size.second;
		mConnectWindow = new ConnectWindow(Hikari::FlashDelegate(this, &ServerMenu::onOk), Hikari::FlashDelegate(this, &ServerMenu::onCancel), "connectionwindow", "serverMenu.swf", width, height, Hikari::Center);
	}

	ServerMenu::~ServerMenu() {
		if (isActivated()) {
			deactivate();
		}
		if (mConnectWindow->isActivated()) {
			GUIManager::getSingletonPtr()->disableOverlay(mConnectWindow);
		}

		delete mCreateButton;
		delete mJoinButton;
		delete mBackButton;
		delete mConnectWindow;
	}

	Hikari::FlashValue ServerMenu::onOk(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		Ogre::String address = args.at(0).getString();

		mListener->onConnect(address, mConnectListener);

		//Disable the input window
		GUIManager::getSingletonPtr()->disableOverlay(mConnectWindow);

		//Activate the rest again
		//re-enable buttons on background
		mCreateButton->ignoreInputs(false);
		mJoinButton->ignoreInputs(false);
		mBackButton->ignoreInputs(false);


		return "success";
	}

	Hikari::FlashValue ServerMenu::onCancel(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		//Hide the connect window
		GUIManager::getSingletonPtr()->disableOverlay(mConnectWindow);

		//re-enable buttons on background
		mCreateButton->ignoreInputs(false);
		mJoinButton->ignoreInputs(false);
		mBackButton->ignoreInputs(false);

		return "success";
	}

	Hikari::FlashValue ServerMenu::onCreate(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		mListener->onCreate();

		return "success";
	}

	Hikari::FlashValue ServerMenu::onJoin(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		//show the connectWindow
		GUIManager::getSingletonPtr()->activateOverlay(mConnectWindow);

		//disable buttons on background
		mCreateButton->ignoreInputs(true);
		mJoinButton->ignoreInputs(true);
		mBackButton->ignoreInputs(true);

		return "success";
	}
}