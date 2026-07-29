#include "EntityManager.h"
#include "PlayerSettings.h"
#include "Lobby.h"
#include "GameEvent.h"
#include "GameEventParser.h"
#include "InitEvent.h"
#include "EntityMapping.h"

#include <OgreLogManager.h>

namespace HovUni {

PlayerSettings::PlayerSettings(Lobby * lobby, unsigned int connID) :
	NetworkEntity(3), mConnID(connID), mUserID(getUniqueID()), mCharacter(0), mHovercraft(0), mPlayerName(""), mLobby(lobby) {

	// Add as network entity
	networkRegister(NetworkIDManager::getServerSingletonPtr(), getClassName(), true);
	mNode->setEventNotification(true, false);
	mNode->dependsOn(lobby->getNetworkNode());

	// Set owner
	mNode->setOwner(connID, true);
}

PlayerSettings::PlayerSettings(Lobby* lobby, const Ogre::String& name) :
	NetworkEntity(3), mConnID(ZCom_Invalid_ID), mUserID(getUniqueID()), mCharacter(0), mHovercraft(0), mPlayerName(name), mLobby(
			lobby) {

	// Add as network entity
	networkRegister(NetworkIDManager::getServerSingletonPtr(), getClassName(), true);
	mNode->setEventNotification(true, false);
}

PlayerSettings::PlayerSettings(Lobby * lobby, ZCom_BitStream* announcementdata, ZCom_ClassID id, ZCom_Control* control) :
	NetworkEntity(3), mConnID(announcementdata->getInt(32)), mUserID(announcementdata->getInt(32)), mCharacter(0), mHovercraft(0),
			mPlayerName(""), mLobby(lobby) {

	// Add as network entity
	networkRegister(id, control);
	mNode->setEventNotification(true, false);
}

PlayerSettings::~PlayerSettings() {
	Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "[PlayerSettings]: Deleting PlayerSettings for " << mPlayerName;
	releaseUniqueID(mUserID);
}

std::string PlayerSettings::getClassName() {
	return "PlayerSettings";
}

void PlayerSettings::setPlayerName(const Ogre::String& name) {
	mPlayerName.assign(name);
}

const Ogre::String& PlayerSettings::getPlayerName() const {
	return mPlayerName;
}

void PlayerSettings::setCharacter(unsigned int character) {
	mCharacter = character;
}

const Ogre::String PlayerSettings::getCharacter() const {
	return EntityMapping::getInstance().getName(EntityMapping::CHARACTER,mCharacter).first;
}

void PlayerSettings::setHovercraft(unsigned int hov) {
	mHovercraft = hov;
}

const Ogre::String PlayerSettings::getHovercraft() const {
	return EntityMapping::getInstance().getName(EntityMapping::HOVERCRAFT,mHovercraft).first;
}

const unsigned int PlayerSettings::getID() const {
	return mUserID;
}

const unsigned int PlayerSettings::getConnID() const {
	return mConnID;
}

void PlayerSettings::setAnnouncementData(ZCom_BitStream* stream) {
	stream->addInt(mConnID, 32);
	stream->addInt(mUserID, 32);
}

void PlayerSettings::parseEvents(eZCom_Event type, eZCom_NodeRole remote_role, ZCom_ConnID conn_id, ZCom_BitStream* stream,
		float timeSince) {
	if (type == eZCom_EventUser) {
		GameEventParser p;
		GameEvent* gEvent = p.parse(stream);

		// Check for an init event if this object is just created
		InitEvent* init = dynamic_cast<InitEvent*> (gEvent);
		if (init) {
			ZCom_BitStream* state = init->getStream();
			mPlayerName = state->getString();
			mHovercraft = state->getInt(4);
			mCharacter = state->getInt(4);
		}

		delete gEvent;
	}

	// A new client received this object so send current state
	if (type == eZCom_EventInit && mNode->getRole() == eZCom_RoleAuthority) {
		ZCom_BitStream* state = new ZCom_BitStream();
		state->addString(mPlayerName.c_str());
		state->addInt(mHovercraft, 4);
		state->addInt(mCharacter, 4);
		sendEventDirect(InitEvent(state), conn_id);
	}
}

void PlayerSettings::setupReplication() {
	//replicate name
	replicateString(&mPlayerName, ZCOM_REPRULE_OWNER_2_AUTH | ZCOM_REPRULE_AUTH_2_PROXY);

	//replicate hovercraft
	replicateUnsignedInt((int*) &mHovercraft, ZCOM_REPRULE_OWNER_2_AUTH | ZCOM_REPRULE_AUTH_2_PROXY, 4);

	//replicate character
	replicateUnsignedInt((int*) &mCharacter, ZCOM_REPRULE_OWNER_2_AUTH | ZCOM_REPRULE_AUTH_2_PROXY, 4);
}

std::set<unsigned int> PlayerSettings::msIDs;
unsigned int PlayerSettings::msLowestPossible = 20;

unsigned int PlayerSettings::getUniqueID() {
	unsigned int i = msLowestPossible;

	while (msIDs.find(i) != msIDs.end()) {
		++i;
	}

	msIDs.insert(i);
	msLowestPossible = i + 1;
	return i;
}

void PlayerSettings::releaseUniqueID(unsigned int id) {
	msIDs.erase(id);
	msLowestPossible = (msLowestPossible < id ? msLowestPossible : id);
}

}
