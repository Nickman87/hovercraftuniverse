#include "HUClient.h"
#include "HUApplication.h"
#include "ClientLoader.h"
#include "EntityRegister.h"
#include "Exception.h"

#include "HovercraftPlayerController.h"
#include "HovercraftAIController.h"
#include "HovercraftLoader.h"

#include "Application.h"
#include "Config.h"
#include "Lobby.h"
#include "PlayerSettings.h"
#include "RaceState.h"
#include "RacePlayer.h"

#include "Start.h"
#include "Finish.h"
#include "Asteroid.h"
#include "Hovercraft.h"
#include "CheckPoint.h"
#include "StartPosition.h"
#include "SpeedBoost.h"
#include "Portal.h"
#include "PowerupSpawn.h"
#include "ResetSpawn.h"
#include "StaticBody.h"

#include "ProgressMonitor.h"
#include <OgreLogManager.h>

namespace HovUni {

HUClient::HUClient(const char* name, unsigned int port) :
	NetworkClient(name, port, "HUClient"), mAddress(name), mSemaphore(0), mFinishedConnecting(false) {
	initialize();
}

HUClient::HUClient() :
	NetworkClient(2376, "HUClient"), mAddress(""), mSemaphore(0), mFinishedConnecting(false) {
	initialize();
}

void HUClient::initialize() {
	//Set chat variables to null
	mChatListener = 0;
	mChatClient = 0;

	// Create and store entity manager
	mEntityManager = EntityManager::getClientSingletonPtr();
	mIDManager = new NetworkIDManager(this);
	EntityRegister::registerAll(*mIDManager);
	ZCom_setUpstreamLimit(0, 0);

	//Create the lobby object
	mLobby = new Lobby(HUApplication::msPreparationLoader);
	mLobby->networkRegisterUnique(mIDManager, Lobby::getClassName());
}

HUClient::~HUClient() {
	delete mChatClient;
}

void HUClient::process(zU32 simulationTimePassed) {
	NetworkClient::process(simulationTimePassed);

	if (mLobby) {
		mLobby->process();
	}

	if (mChatClient) {
		mChatClient->process();
	}
}

void HUClient::disconnect(const std::string& reason) {
	NetworkClient::disconnect(reason);
	if (mChatClient) {
		mChatClient->disconnect(reason);
	}
}

unsigned int HUClient::getID() const {
	return mID;
}

void HUClient::onConnectResult(eZCom_ConnectResult result, ZCom_BitStream& extra) {

	Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "[HUClient]: received connection result";

	if (result == eZCom_ConnAccepted) {
		// Connection accepted, so request zoid level
		ZCom_requestDownstreamLimit(mConnID, 60, 600);
		ZCom_requestZoidMode(mConnID, 1);

		// Get the unique ID
		mID = extra.getInt(sizeof(ZCom_ConnID) * 8);
		Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "[HUClient\\" << mID << "]: My unique ID is " << mID;

		//Start the chat client
		std::string nickname = Application::getConfig()->getValue<std::string>("Player", "PlayerName", "UnnamedPlayer");
		if (mAddress == "") {
			mChatClient = new ChatClient(nickname);
		} else {
			mChatClient = new ChatClient(nickname, mAddress.c_str());
		}
		mChatClient->connect(0);

		//Check if we already have a listener
		if (mChatListener != 0) {
			mChatClient->addListener(mChatListener);
		}
	}

	mFinishedConnecting = true;
	//Notify our listeners
	mSemaphore.post();
}

void HUClient::onDisconnect(eZCom_CloseReason reason, ZCom_BitStream& extra) {
	// Connection closed
}

void HUClient::onDataReceived(ZCom_BitStream& data) {
	// Data received
}

void HUClient::onZoidResult(eZCom_ZoidResult result, zU8 new_level, ZCom_BitStream& reason) {
	if (result == eZCom_ZoidEnabled) {
		// Requested zoid level was confirmed
	} else {
		// Requested zoid level denied
	}
}

void HUClient::onNodeDynamic(ZCom_ClassID requested_class, ZCom_BitStream* announcedata, eZCom_NodeRole role, ZCom_NodeID net_id) {
	// Receive and create the entity
	Ogre::String name("");

	// Debug output
	Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "[HUClient\\" << mID << "]: Creating object of type "
			<< mIDManager->getName(requested_class);

	//Lobby
	if (requested_class == mIDManager->getID(PlayerSettings::getClassName())) {
		// Player
		PlayerSettings * ent = new PlayerSettings(mLobby, announcedata, requested_class, this);
		ent->processEvents(0.0f);

		// Let's check if this is our own PlayerSettings object
		if (role == eZCom_RoleOwner) {
			// Put our data in it
			Config* conf = Application::getConfig();
			ent->setPlayerName(conf->getValue<std::string>("Player", "PlayerName", ""));
			ent->setCharacter(conf->getValue<int>("Player", "Character", 0));
			ent->setHovercraft(conf->getValue<int>("Player", "Hovercraft", 0));
		}

		// Network register is done in constructor of player settings
		//Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream()
		//		<< "[HUClient]: adding player to lobby";
		mLobby->addPlayer(ent, ent->getConnID() == mID);
	} else if (requested_class == mIDManager->getID(RaceState::getClassName())) {
		RaceState* ent = new RaceState(mLobby, HUApplication::msPreparationLoader, announcedata, requested_class, this);
		mLobby->setRaceState(ent);

		// Monitor the loading of the hovercrafts
		ProgressMonitor::getSingletonPtr()->addTask("Loading hovercraft entities.", ent->getPlayers().size());
		ProgressMonitor::getSingletonPtr()->addTask("Loading hovercraft.", ent->getPlayers().size());
	} else if (requested_class == mIDManager->getID(RacePlayer::getClassName())) {
		if (!mLobby->getRaceState()) {
			Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "[HUClient]: Error - no race state";
			THROW(NetworkException, "RaceState not received yet");
		}
		RacePlayer* ent = new RacePlayer(mLobby, announcedata, requested_class, this);
		mLobby->getRaceState()->addPlayer(ent, ent->getSettings()->getID() == mLobby->getOwnPlayer()->getID());
	}

	//Entities
	if (requested_class == mIDManager->getID(Asteroid::getClassName())) {
		Asteroid * ent = new Asteroid(announcedata);
		ent->networkRegister(requested_class, this);
		mEntityManager->registerEntity(ent);
		name = ent->getName();
		ent->setRaceState(mLobby->getRaceState());
	} else if (requested_class == mIDManager->getID(CheckPoint::getClassName())) {
		CheckPoint * ent = new CheckPoint(announcedata);
		ent->networkRegister(requested_class, this);
		mEntityManager->registerEntity(ent);
		name = ent->getName();
		ent->setRaceState(mLobby->getRaceState());
	} else if (requested_class == mIDManager->getID(Start::getClassName())) {
		Start * ent = new Start(announcedata);
		ent->networkRegister(requested_class, this);
		mEntityManager->registerEntity(ent);
		name = ent->getName();
		ent->setRaceState(mLobby->getRaceState());
	} else if (requested_class == mIDManager->getID(StartPosition::getClassName())) {
		StartPosition * ent = new StartPosition(announcedata);
		ent->networkRegister(requested_class, this);
		mEntityManager->registerEntity(ent);
		name = ent->getName();
		ent->setRaceState(mLobby->getRaceState());
	} else if (requested_class == mIDManager->getID(Finish::getClassName())) {
		Finish * ent = new Finish(announcedata);
		ent->networkRegister(requested_class, this);
		mEntityManager->registerEntity(ent);
		name = ent->getName();
		ent->setRaceState(mLobby->getRaceState());
	} else if (requested_class == mIDManager->getID(ResetSpawn::getClassName())) {
		ResetSpawn * ent = new ResetSpawn(announcedata);
		ent->networkRegister(requested_class, this);
		mEntityManager->registerEntity(ent);
		name = ent->getName();
		ent->setRaceState(mLobby->getRaceState());
	} else if (requested_class == mIDManager->getID(PowerupSpawn::getClassName())) {
		PowerupSpawn * ent = new PowerupSpawn(announcedata);
		ent->networkRegister(requested_class, this);
		mEntityManager->registerEntity(ent);
		name = ent->getName();
		ent->setRaceState(mLobby->getRaceState());
	} else if (requested_class == mIDManager->getID(SpeedBoost::getClassName())) {
		SpeedBoost * ent = new SpeedBoost(announcedata);
		ent->networkRegister(requested_class, this);
		mEntityManager->registerEntity(ent);
		name = ent->getName();
		ent->setRaceState(mLobby->getRaceState());
	} else if (requested_class == mIDManager->getID(Portal::getClassName())) {
		Portal * ent = new Portal(announcedata);
		ent->networkRegister(requested_class, this);
		mEntityManager->registerEntity(ent);
		name = ent->getName();
		ent->setRaceState(mLobby->getRaceState());
	} else if (requested_class == mIDManager->getID(StaticBody::getClassName())) {
		StaticBody * ent = new StaticBody(announcedata);
		ent->networkRegister(requested_class, this);
		mEntityManager->registerEntity(ent);
		name = ent->getName();
		ent->setRaceState(mLobby->getRaceState());
	} else if (requested_class == mIDManager->getID(Hovercraft::getClassName())) {
		Hovercraft * ent = new Hovercraft(announcedata);
		ent->setRaceState(mLobby->getRaceState());
		ent->updateLabel();

		// Monitor the loading of the hovercrafts
		ProgressMonitor::getSingletonPtr()->updateTask("Loading hovercraft entities.");

		if (role == eZCom_RoleOwner) {
			Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "[HUClient]: Received own hovercraft";
			ent->setController(new HovercraftPlayerController());
		}

		ent->networkRegister(requested_class, this);
		mEntityManager->registerEntity(ent);

		if (role == eZCom_RoleOwner) {
			mEntityManager->trackEntity(ent->getName());
		}

		// Visualize the hovercraft
		HovercraftLoader hovLoader(HUApplication::msSceneMgr, ent->getName());
		hovLoader.load(ent->getDisplayName() + ".scene");
	}

	if (!name.empty()) {
		// Now that we have created the entity, notify the client preparation loader of the arrival
		HUApplication::msPreparationLoader->update(name);
	}

}

void HUClient::setChatListener(ChatListener* listener) {
	mChatListener = listener;
	if (mChatClient != 0) {
		mChatClient->addListener(mChatListener);
	}
}

void HUClient::removeChatListener(ChatListener* listener) {
	if (mChatListener == listener) {
		mChatListener = 0;
	}
	if (mChatClient != 0) {
		mChatClient->removeListener(listener);
	}
}

void HUClient::wait() {
	mSemaphore.wait();
}

void HUClient::timed_wait(const boost::posix_time::ptime & abs_time) {
	mSemaphore.timed_wait(abs_time);
}

}
