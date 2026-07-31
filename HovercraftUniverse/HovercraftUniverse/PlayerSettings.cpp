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
			// Always consume the stream in full, regardless of whether we
			// apply it below -- the bitstream's read cursor must advance by
			// the exact amount that was written (see
			// zoidcom-original-semantics.md's onDataReceived() note: "forward
			// the bitstream by the exact amount of bytes originally sent").
			Ogre::String newName = state->getString();
			unsigned int newHov = state->getInt(4);
			unsigned int newChar = state->getInt(4);
			// name/hovercraft/character are all declared OWNER_2_AUTH in
			// setupReplication() below -- this owner's own node is the
			// authoritative source for them, never the authority. The
			// authority's InitEvent snapshot reflects whatever it has on
			// file *at the moment this connection linked*, which for a
			// brand-new per-connection node is still the constructor's
			// blank default (see PlayerSettings(Lobby*, unsigned int)):
			// the owner hasn't had a chance to push its real values up via
			// replication yet. Applying that stale snapshot to the owner's
			// own copy would silently erase whatever the owner just set
			// locally (e.g. HUClient::onNodeDynamic() applying the
			// configured player name/character/hovercraft) with no way to
			// ever correct it afterwards, since AUTH_2_PROXY deliberately
			// excludes Owner from being overwritten by later replication
			// ticks either. A genuine proxy (any other client's view of
			// this player) has no such local authority and must apply the
			// snapshot to have anything to show at all.
			if (mNode->getRole() != eZCom_RoleOwner) {
				mPlayerName = newName;
				mHovercraft = newHov;
				mCharacter = newChar;
			}
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
