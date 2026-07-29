#include "PlayerSettingsInterceptor.h"
#include "Ogre.h"

namespace HovUni {

PlayerSettingsInterceptor::PlayerSettingsInterceptor(PlayerSettings* playerSettings,
		PlayerSettingsListener* listener) :
	mPlayerSettings(playerSettings), mListener(listener), mStatus(true) {
	//Register ourselves to the playersettings
	mPlayerSettings->setReplicationInterceptor(this);
}

void PlayerSettingsInterceptor::setStatus(bool active) {
	mStatus = active;
}

bool PlayerSettingsInterceptor::inPreUpdate(ZCom_Node *_node, ZCom_ConnID _from,
		eZCom_NodeRole _remote_role) {
	//Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "[PlayerSettingsInterceptor]: " << "We intercepted replication, old values: " << mPlayerSettings->getPlayerName() << " " << mPlayerSettings->getCharacter() << " " << mPlayerSettings->getHovercraft();
	return true;
}

bool PlayerSettingsInterceptor::inPreUpdateItem(ZCom_Node *_node, ZCom_ConnID _from,
		eZCom_NodeRole _remote_role, ZCom_Replicator *_replicator, zU32 _estimated_time_sent) {
	return true;
}

void PlayerSettingsInterceptor::inPostUpdate(ZCom_Node *_node, ZCom_ConnID _from,
		eZCom_NodeRole _remote_role, zU32 _rep_bits, zU32 _event_bits, zU32 _meta_bits) {
	//Incoming data has updated the node.
	//We don't really care about the parameters for now, just notify our listener
	updatePlayer();
}

void PlayerSettingsInterceptor::outPostUpdate(ZCom_Node *_node, ZCom_ConnID _to,
		eZCom_NodeRole _remote_role, zU32 _rep_bits, zU32 _event_bits, zU32 _meta_bits) {
	//Incoming data has updated the node.
	//We don't really care about the parameters for now, just notify our listener
	updatePlayer();
}

void PlayerSettingsInterceptor::updatePlayer() {
	if (mStatus) {
		if (mListener) {
			mListener->onPlayerUpdate(mPlayerSettings->getID(), mPlayerSettings->getPlayerName(),
					mPlayerSettings->getCharacter(), mPlayerSettings->getHovercraft());
		}
	}
}


}
