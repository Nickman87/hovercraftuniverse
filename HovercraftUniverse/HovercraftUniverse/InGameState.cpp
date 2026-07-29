#include "InGameState.h"
#include "Application.h"
#include "CheckPoint.h"
#include "Finish.h"
#include "Hovercraft.h"
#include "HovercraftRepresentation.h"
#include "ProgressMonitor.h"
#include "Start.h"
#include <HovSound.h>

namespace HovUni {
	InGameState::InGameState(HUClient* client, RaceState* raceState, TiXmlElement* HUDConfig)
			: mHUClient(client), mTimeLapsed(0), mContinue(true), mRaceState(raceState), mLoader(0), mCountdownFadeout(-1), mUpdateListener(false), mCleaningUp(false), mRun(false), mResults(0) {

		mHud = new HUD(HUDConfig, Hikari::FlashDelegate(this, &InGameState::onChat));
		mEntityManager = EntityManager::getClientSingletonPtr();
		mRepresentationManager = RepresentationManager::getSingletonPtr();
		mRaceState->addListener(this);
	}

	Hikari::FlashValue InGameState::onChat(Hikari::FlashControl* caller, const Hikari::Arguments& args) {
		Ogre::String chatText = args.at(0).getString();

		if (chatText != "") {
			mHUClient->getChat()->sendText(chatText);
			Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "[InGameState]: " << "Sending chat message: " << args.at(0).getString();
		}

		return "success";
	}

	////////////////////////////////////////////
	//	 ProgressMonitorListener functions	  //
	////////////////////////////////////////////

	void InGameState::updateProgress(double progress) {
		if (mLoader) {
			//Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "[InGameState]: OMFG? " << (float) (progress * 100.0f);
			mLoader->setLoaded((float) (progress * 100.0f), "Loading...");
		}
	}

	////////////////////////////////////////////
	//			RaceStateListener functions	  //
	////////////////////////////////////////////

	void InGameState::onStateChange(RaceState::States state) {
		switch (state) {
			case RaceState::INITIALIZING:
				//People are still creating the racestate, start showing the loading screen
				//We don't want the listener position to be updated
				mUpdateListener = false;

				break;
			case RaceState::LOADING: {
				//We are actually loading, make sure we are monitoring everything
				//TODO make it get real values
				//mLoader->setLoaded(50.0f, "Loading...");

				break;
			}
			case RaceState::INTRO: {
				Entity* currEnt = EntityManager::getClientSingletonPtr()->getTrackedEntity();
				mRepresentationManager->getGameViews().at(0)->getCamera()->setPosition(currEnt->getPosition(), currEnt->getOrientation());

				//Everyone is done loading, go ingame and show the intro!
				mGUIManager->disableOverlay(mLoader);

				//TODO: Show cool intro?
				break;
			}
			case RaceState::COUNTDOWN: {
				//Make sure the hud is activated
				mHud->activate();
				//Register for chat events
				mHUClient->setChatListener(mHud);

				//Show the countdown
				mGUIManager->activateOverlay(mCountdown);
				mCountdown->start(5000);

				mMutex.lock();
				
				//Activate the engines
				std::vector<EntityRepresentation*> entreps = RepresentationManager::getSingletonPtr()->getEntityRepresentations();
				for (std::vector<EntityRepresentation*>::iterator i = entreps.begin(); i != entreps.end(); i++) {
					HovercraftRepresentation * hov = dynamic_cast<HovercraftRepresentation *> (*i);
					if (hov != 0) {
						SoundManager::getSingletonPtr()->registerEmitter(hov);
						hov->startSound();
						hov->setVolume(0.8f);
					}
				}

				HovercraftRepresentation* me = (HovercraftRepresentation*) mRepresentationManager->getTrackedEntityRepresentation();
				me->setVolume(0.4f);
				
				mMutex.unlock();

				//Start updating the sound listener
				mUpdateListener = true;

				break;
			}
			case RaceState::RACING: {
				mCountdownFadeout = 3;
				//Countdown has finished, race has started!
				mHud->startLapTimer();
				mHud->setCurrentPosition(mRaceState->getOwnPlayer()->getPosition());
				mHud->setNumberOfPlayers(mRaceState->getPlayers().size());
				mFinished = false;

				break;
			}
			case RaceState::FINISHING: {
				mFinished = false;
				//We have finished, go freestyle!
				//mRaceState->
				mGUIManager->activateOverlay(mCountdown);
				mCountdown->start(mRaceState->getCountdown());

				break;
			}
			case RaceState::CLEANUP: {
				//Cleaning up and going back to the lobby

				//Stop updating the sound listener
				mUpdateListener = false;

				//Stop some things
				mCleaningUp = true;

				break;
			}
		}
	}

	void InGameState::onPositionChange(RacePlayer* player) {
		if (mHud->isActivated() && player->getSettings()->getID() == mRaceState->getOwnPlayer()->getSettings()->getID()) {
			Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "[InGameState]: Own position: " << mRaceState->getOwnPlayer()->getPosition();
			mHud->setCurrentPosition(mRaceState->getOwnPlayer()->getPosition());
			mHud->setNumberOfPlayers(mRaceState->getPlayers().size());
		}
	}

	void InGameState::onFinish(RacePlayer* player) {
		

		//mResults->addPlayer(player->getPosition(), player->getSettings()->getPlayerName(), player->getCheckpoint(player->getLastCheckpoint()));
	}

	////////////////////////////////////////////
	//			BasicGameState functions	  //
	////////////////////////////////////////////

	void InGameState::activate() {
		// Either this one may be entered
		//boost::mutex::scoped_lock l(mMutex);

		mRepresentationManager->getGameViews().at(0)->getCamera()->reinitialize();

		//Make sure we activate all the key actions
		mInputManager->getKeyManager()->setActive();

		//Remove cursor
		mGUIManager->showCursor(false);

		//Make sure the hud is deactivated
		mHud->deactivate();

		if (mLoader == 0) {
			mLoader = new LoadingOverlay("Loader", "loader.swf", mGUIManager->getResolutionWidth(), mGUIManager->getResolutionHeight(), Hikari::Center);
			mLoader->setNoBorders(true);
			mGUIManager->activateOverlay(mLoader);
			mLoader->setLoaded(0.0f, "Initializing");
			ProgressMonitor::getSingletonPtr()->addListener(this);
		}

		// Create skies
		mSkyX = new SkyX::SkyX(Application::msSceneMgr, mRepresentationManager->getGameViews().at(0)->getCamera()->getCamera());
		mSkyX->create();
		//mSkyX->getCloudsManager()->add(SkyX::CloudLayer::Options());
		SkyX::AtmosphereManager::Options skyXOptions = mSkyX->getAtmosphereManager()->getOptions();
		skyXOptions.Time.x = 23.0f;
		skyXOptions.Time.y = 10.0f;//sunRise
		skyXOptions.Time.z = 10.0f;//sunSet (== sunrise so the sun NEVER SETS ;))
		skyXOptions.WaveLength = Ogre::Vector3(0.1f, 0.1f, 0.1f);
		mSkyX->getAtmosphereManager()->setOptions(skyXOptions);

		
		std::pair<int, int> size = GUIManager::getSingletonPtr()->scale(200, 100, 400, 200);
		mCountdown = new Countdown("Countdown", "countdown.swf", size.first, size.second, Hikari::Center);

		//Switch music
		mSoundManager->startAmbient(MUSICCUE_HOVSOUND_BACKGROUND_NORMAL);

		// Start drawing
		mRun = true;
	}

	void InGameState::disable() {
		// Or this one
		//boost::mutex::scoped_lock l(mMutex);

		// Stop drawing
		mRun = false;

		// Clean up skies
		mSkyX->remove();

		//Deregister for chatevents
		mHUClient->removeChatListener(mHud);
		mGUIManager->disableOverlay(mCountdown);

		//Remove the hud
		if (mHud != 0) {
			mHud->deactivate();
		}

		if (mResults != 0) {
			mGUIManager->disableOverlay(mResults);
		}

		//Restore cursor
		mGUIManager->showCursor(true);

		if (mLoader != 0) {
			ProgressMonitor::getSingletonPtr()->removeListener(this);
			mGUIManager->disableOverlay(mLoader);
		}
		delete mLoader;
		mLoader = 0;

		delete mCountdown;
		mCountdown = 0;

		mRaceState->removeListener(this);

		// Remove representation managers
		std::vector<EntityRepresentation*> entreps = RepresentationManager::getSingletonPtr()->getEntityRepresentations();
        for (std::vector<EntityRepresentation*>::iterator i = entreps.begin(); i != entreps.end(); i++) {
			RepresentationManager::getSingletonPtr()->removeEntityRepresentation((*i)->getEntity()->getName());
            delete (*i);
        }

		// Remove entities
		std::vector<Entity*> ents = EntityManager::getClientSingletonPtr()->getEntities(EntityManager::ALL);
        for (std::vector<Entity*>::iterator i = ents.begin(); i != ents.end(); i++) {
            EntityManager::getClientSingletonPtr()->releaseEntity((*i)->getName());
            delete (*i);
        }

		// Remove everything
		Application::msSceneMgr->destroyAllAnimations();
		Application::msSceneMgr->destroyAllAnimationStates();
		Application::msSceneMgr->destroyAllBillboardChains();
		Application::msSceneMgr->destroyAllBillboardSets();
		Application::msSceneMgr->destroyAllEntities();
		Application::msSceneMgr->destroyAllMovableObjects();
		Application::msSceneMgr->destroyAllInstancedGeometry();
		Application::msSceneMgr->destroyAllLights();
		Application::msSceneMgr->destroyAllEntities();
		Application::msSceneMgr->getRootSceneNode()->removeAndDestroyAllChildren();

		//Switch music
		mSoundManager->stopAmbient(MUSICCUE_HOVSOUND_BACKGROUND_NORMAL);
		mSoundManager->startAmbient(MUSICCUE_HOVSOUND_MENU);
	}

	bool InGameState::frameStarted(const Ogre::FrameEvent & evt) {
		// Or this one
		//boost::mutex::scoped_lock l(mMutex);

		if (mRun) {
			bool result = true;

			mTimeLapsed += evt.timeSinceLastFrame;

			// Update entity manager
			mEntityManager->updateEntities(evt.timeSinceLastFrame);

			// Process the client
			if (mTimeLapsed > 0.016f) {
				// Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "Client start input output process";
				mHUClient->process((int) (mTimeLapsed * 1000.0f));
				// Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "Client ends input output process";
				mTimeLapsed = 0.0f;
			}

			if (!mCleaningUp) {
				//Check if we need to synchronise the countdown
				if ((mRaceState != 0) && ((mRaceState->getState() == RaceState::COUNTDOWN) || (mRaceState->getState() == RaceState::FINISHING))) {
					//std::cout << "TIME LEFT: " << mRaceState->getCountdown() << std::endl;
					mCountdown->resync(mRaceState->getCountdown());
				}

				if (!mFinished) {
					if ((mRaceState != 0) && (mRaceState->getState() == RaceState::FINISHING)) {
						RacePlayer* ownPlayer = mRaceState->getOwnPlayer();
						if (ownPlayer->isFinished()) {
							mHud->stopLapTimer();
							long milliseconds = ownPlayer->getCheckpoint(ownPlayer->getLastCheckpoint());

							int minutes = milliseconds / (1000 * 60);
							int seconds = (milliseconds - minutes * (1000 * 60)) / 1000;
							int hundredseconds = (milliseconds - seconds * 1000 - minutes * (60 * 1000)) / 10;

							mHud->setLapTimer(minutes, seconds, hundredseconds);
							mFinished = true;

							if (mResults == 0) {
								int height = GUIManager::getSingletonPtr()->getResolutionHeight() / 2; //We want to fill half the screen
								float scale = (height * 1.0f) / 400.0;
								int width = (int) (550 * scale);
								mResults = new Results("ResultsOverlay", "raceresults.swf", width, height, Hikari::TopCenter);
								if (mHud->isActivated()) {
									mHud->hideDirection();
								}
								mGUIManager->activateOverlay(mResults);
							}
						}
					}
				}

				if (mFinished) {
					//Add finished players to the list
					const RaceState::playermap::list_type players = mRaceState->getPlayers();

					RaceState::playermap::const_iterator it;
					
					for (it = players.begin(); it != players.end(); ++it) {
						if ((*it).second->isFinished()) {
							mResults->addPlayer((*it).second->getPosition(), (*it).second->getSettings()->getPlayerName(), (*it).second->getCheckpoint((*it).second->getLastCheckpoint()));
						}
					}
				}
			} else {
				//We want to switch state!
				mRaceState->removeListener(this);
				mManager->switchState(GameStateManager::LOBBY);
			}

			//Check if we need to remove the countdown overlay
			if (mCountdownFadeout != -1) {
				mCountdownFadeout -= evt.timeSinceLastFrame;

				if (mCountdownFadeout <= 0) {
					mGUIManager->disableOverlay(mCountdown);
					mCountdownFadeout = -1;
				}
			}

			//Update the sky
			mSkyX->update(evt.timeSinceLastFrame);

			// Update representation manager
			if (mRun) {
				mRepresentationManager->drawGameViews(evt.timeSinceLastFrame);
			}

			//Update the HUD
			if (!mCleaningUp && mHud->isActivated()) {
				updateHud();
			}

			//We are using a GUI, so update it
			mGUIManager->update();

			//We have sound, update it
			if (mUpdateListener) {
				//Get current entity
				Entity* currEntity = mEntityManager->getTrackedEntity();
				Ogre::SceneManager::CameraIterator it = mRepresentationManager->getSceneManager()->getCameraIterator();
				Ogre::Camera* cam = it.getNext();
				//std::cout << "Setting listener to pos: " << currEntity->getPosition() << std::endl;
				mSoundManager->updateListenerPosition(&currEntity->getPosition(), &currEntity->getVelocity(), &currEntity->getOrientation());
			}

			mMutex.lock();
			mSoundManager->update();
			mMutex.unlock();

			// Return whether to continue
			return (mContinue && result);
		}

		// Return false
		return mContinue;
	}

	bool InGameState::mouseMoved(const OIS::MouseEvent & e) {
		bool result = true;

		//We are using a GUI, so update it
		result = result && mGUIManager->mouseMoved(e);

		return result;
	}

	bool InGameState::mousePressed(const OIS::MouseEvent & e, OIS::MouseButtonID id) {
		bool result = true;

		//We are using a GUI, so update it
		result = result && mGUIManager->mousePressed(e, id);

		return result;
	}

	bool InGameState::mouseReleased(const OIS::MouseEvent & e, OIS::MouseButtonID id) {
		bool result = true;

		//We are using a GUI, so update it
		result = result && mGUIManager->mouseReleased(e, id);

		return result;
	}

	bool InGameState::keyPressed(const OIS::KeyEvent & e) {
		bool result = true;

		OIS::Keyboard * keyboard = InputManager::getSingletonPtr()->getKeyboard();

		switch (e.key) {
			case OIS::KC_ESCAPE:
			case OIS::KC_LMENU:
			case OIS::KC_RMENU:
			case OIS::KC_F4:
				// Check whether right combinations are pressed concurrently
				if (keyboard->isKeyDown(OIS::KC_ESCAPE) ||
					(keyboard->isKeyDown(OIS::KC_LMENU) && keyboard->isKeyDown(OIS::KC_RMENU) && keyboard->isKeyDown(OIS::KC_F4))) {
					// Stop rendering
					mContinue = false;
				}
				break;
			case OIS::KC_RETURN:
				//Focus the chat
				if (!mHud->isChatFocused()) {
					mHud->focusChat();
					//We don't want any crazy input keys
					mInputManager->getKeyManager()->setInactive();
				} else {
					//Defocus the chat
					mHud->defocusChat();
					//We don't want any crazy input keys
					mInputManager->getKeyManager()->setActive();
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

	bool InGameState::keyReleased(const OIS::KeyEvent & e) {
		bool result = true;

		//We are using a GUI, so update it
		result = result && mGUIManager->keyReleased(e);

		return result;
	}

	void InGameState::updateHud() {
		//Update hud objects to new values
		//Current entity
		Entity* currEnt = EntityManager::getClientSingletonPtr()->getTrackedEntity();

		RacePlayer* myPlayer = mRaceState->getOwnPlayer();

		//std::cout << "WE NEED TO POINT TO: " << (int)myPlayer->getLastCheckpoint() << std::endl;

		Ogre::Vector3 goal = (currEnt->getPosition() + currEnt->getOrientation());

		//Find the correct checkpoint
		std::vector<Entity*> checkpoints = EntityManager::getServerSingletonPtr()->getEntities(CheckPoint::CATEGORY);
		for (unsigned int i = 0; i < checkpoints.size(); ++i) {
			CheckPoint * checkpoint = dynamic_cast<CheckPoint *> (checkpoints[i]);
			if (checkpoint->getNumber() == (myPlayer->getLastCheckpoint() + 1)) {
				goal = checkpoint->getPosition();
				break;
			}

			//If we get here, we didn't find it, point to the finish!
			std::vector<Entity*> finishes = EntityManager::getServerSingletonPtr()->getEntities(Finish::CATEGORY);
			if (finishes.size() >= 1) {
				Finish * finish = dynamic_cast<Finish *> (finishes[0]);
				goal = finish->getPosition();
			}
		}

		if (currEnt != 0) {
			Hovercraft* hov = (Hovercraft*) currEnt;

			mHud->updateDirection(currEnt->getOrientation(), goal - currEnt->getPosition(), currEnt->getUpVector());
			mHud->updateSpeed( fabs((hov->getSpeed() / hov->getMaximumSpeed()) * 100.0f) );
		}
	}
}
