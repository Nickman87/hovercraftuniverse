#include "MainMenuState.h"
#include "InGameState.h"
#include "LobbyState.h"
#include "HUClient.h"
#include <tinyxml/tinyxml.h>
#include <boost/thread/thread_time.hpp>
#include <HovSound.h>
#include <NMessageBox.h>
// Test affordance (revival Phase B, docs/porting/phase-b-plan.md): needed
// for Application::getAutoConnect()/getAutoConnectHost()/getAutoConnectPort().
#include "Application.h"

namespace HovUni {
	MainMenuState::MainMenuState() : mMenu(0), mContinue(true), mLastGUIUpdate(-1), mConnectionThread(0), mConnectionFinished(false), mAutoConnectTriggered(false) {
	}

	MainMenuState::~MainMenuState() {
		delete mConnectionThread;
		delete mMenu;
	}

	void MainMenuState::onConnect(const Ogre::String& address, ConnectListener* listener) {
		//Connect to the given address
		///////////////////////////////////////////
		///////////////////////////////////////////
		//TODO: Parse IP and Port?
		HUClient* mClient = new HUClient(address.c_str());
		LobbyState* newState = new LobbyState(mClient);

		//Store the new state
		mManager->addGameState(GameStateManager::LOBBY, newState);

		//Store the listener
		mConnectionFinished = false;
		mConnectListener = listener;

		//Try connecting
		delete mConnectionThread;
		mConnectionThread = new ClientConnectThread(mClient, this);
		mConnectionThread->start();

		Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "[HUClient]: Connection thread created.";
	}

	void MainMenuState::onConnectFinish(bool success) {
		//Store this result
		mConnectionResult = success;
		mConnectionFinished = true;
	}

	void MainMenuState::onCreate() {
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
			onConnect("localhost", mMenu);
		
			Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "[MainMenu]: create game finished";

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
	}

	void MainMenuState::finishConnect() {
		//Delete the connection thread
		delete mConnectionThread;
		mConnectionThread = 0;

		//Deactivate our overlay
		mMenu->deactivate();
		mManager->switchState(GameStateManager::LOBBY);
	}

	Hikari::FlashValue MainMenuState::onQuit(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		mContinue = false;

		return Hikari::FlashValue();
	}

	void MainMenuState::activate() {
		//We don't want any crazy input keys
		mInputManager->getKeyManager()->setInactive();

		//Creat the MainMenu object
		if (mMenu != 0) {
			delete mMenu;
		}
		mMenu = new MainMenu(this, Hikari::FlashDelegate(this, &MainMenuState::onQuit));

		//Try and move the mouse to the center of the screen
		mInputManager->moveMouseTo(mGUIManager->getResolutionWidth() / 2, mGUIManager->getResolutionHeight() / 2);

		//Make sure we have a cursor
		mGUIManager->showCursor(true);

		//Activate the menu overlay
		mMenu->activate();

		mSoundManager->startAmbient(MUSICCUE_HOVSOUND_MENU);
	}

	void MainMenuState::disable() {
		//Deactivate the menu overlay
		mMenu->deactivate();

		//Delete the menu overlay
		//delete mMenu;
	}

	bool MainMenuState::frameStarted(const Ogre::FrameEvent & evt) {
		// Test affordance (revival Phase B, docs/porting/phase-b-plan.md): a
		// --autoconnect command-line flag lets a two-process test harness
		// skip the Flash main menu entirely, so tests don't have to click
		// through GUI buttons to reach a race. This has to be triggered from
		// here (the first frameStarted() tick) rather than the constructor,
		// because onConnect() below needs mMenu and the state manager to be
		// fully built -- exactly the same reason the connection-result poll
		// just below already lives in frameStarted() rather than a callback.
		// Without --autoconnect, Application::getAutoConnect() is false and
		// this whole block is a no-op: behaviour is unchanged from before
		// this affordance existed.
		if (!mAutoConnectTriggered) {
			mAutoConnectTriggered = true;

			if (Application::getAutoConnect()) {
				const Ogre::String& host = Application::getAutoConnectHost();
				unsigned int port = Application::getAutoConnectPort();

				Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream()
					<< "[Autoconnect]: connecting to " << host << ":" << port;

				// Reuse the exact same onConnect()/mMenu pairing that the
				// "Join game" button uses (see ServerMenu::onOk and
				// MainMenu::onSingleplayer) so this harness exercises the
				// real production connect path end-to-end, instead of
				// reimplementing it.
				onConnect(host, mMenu);
			}
		}

		//Check if we have a connection result
		if (mConnectionFinished) {
			mConnectListener->onConnectFinish(mConnectionResult);
			mConnectListener = 0;
			mConnectionFinished = false;
		}

		bool result = true;

		mLastGUIUpdate += evt.timeSinceLastFrame;

		//50 FPS
		if (mLastGUIUpdate > (1.0f / 50.0f) || mLastGUIUpdate < 0) {
			//We are using a GUI, so update it
			mGUIManager->update();
			mLastGUIUpdate = 0.0f; //Reset
		}

		//We have sound, update it
		mSoundManager->update();

		return (result && mContinue);
	}

	bool MainMenuState::mouseMoved(const OIS::MouseEvent & e) {
		bool result = true;

		//We are using a GUI, so update it
		result = result && mGUIManager->mouseMoved(e);

		return result;
	}

	bool MainMenuState::mousePressed(const OIS::MouseEvent & e, OIS::MouseButtonID id) {
		bool result = true;


		//We are using a GUI, so update it
		result = result && mGUIManager->mousePressed(e, id);
		
		return result;
	}

	bool MainMenuState::mouseReleased(const OIS::MouseEvent & e, OIS::MouseButtonID id) {
		bool result = true;

		//We are using a GUI, so update it
		result = result && mGUIManager->mouseReleased(e, id);
		
		return result;
	}

	bool MainMenuState::keyPressed(const OIS::KeyEvent & e) {
		bool result = true;

		OIS::Keyboard * keyboard = InputManager::getSingletonPtr()->getKeyboard();

		switch (e.key) {
			case OIS::KC_ESCAPE:
			case OIS::KC_F4:
				// Check whether right combinations are pressed concurrently
				if ((keyboard->isKeyDown(OIS::KC_ESCAPE)) || 
					((keyboard->isKeyDown(OIS::KC_LMENU) || keyboard->isKeyDown(OIS::KC_RMENU)) && keyboard->isKeyDown(OIS::KC_F4))
					) {
					// Stop rendering
					mContinue = false;			
				}

				break;
			default:
				// Do nothing
				break;
		}

		//We are using a GUI, so update it
		result = result && mGUIManager->keyPressed(e);
		
		return result;
	}

	bool MainMenuState::keyReleased(const OIS::KeyEvent & e) {
		bool result = true;

		//We are using a GUI, so update it
		result = result && mGUIManager->keyReleased(e);
		
		return result;
	}
}
