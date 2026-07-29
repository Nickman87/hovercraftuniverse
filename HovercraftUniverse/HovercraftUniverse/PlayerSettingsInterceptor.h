#ifndef _PLAYERSETTINGSINTERCEPTOR_H
#define _PLAYERSETTINGSINTERCEPTOR_H

#include "PlayerSettings.h"
#include "PlayerSettingsListener.h"
#include <zoidcom/zoidcom.h>

namespace HovUni {

/**
 * Interceptor for the player settings. This allows to register callbacks in the
 * PlayerSettings class.
 *
 * @author Nick De Frangh
 */
class PlayerSettingsInterceptor: public ZCom_NodeReplicationInterceptor {
private:
	/** The playerSettings object linked to this interceptor */
	PlayerSettings* mPlayerSettings;

	/** The listener who wants to get updates */
	PlayerSettingsListener* mListener;

	/** The current status of the interceptor */
	bool mStatus;

public:
	/**
	 * Constructor for a playersettingsInterceptor
	 *
	 * @param playerSettings The object to register this listener to
	 * @param listener The listener who wants to receive callbacks
	 */
	PlayerSettingsInterceptor(PlayerSettings* playerSettings, PlayerSettingsListener* listener);

	/**
	 * Set the status of the interceptor to active or inactive.
	 * When you make the interceptor inactive, no more callbacks will be generated
	 *
	 * @param active True to active the intercepter, false to deactivate it
	 */
	void setStatus(bool active);

	/**
	 * @inheritDoc
	 */
	bool inPreUpdate(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role);

	/**
	 * @inheritDoc
	 */
	bool inPreUpdateItem(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role,
			ZCom_Replicator *_replicator, zU32 _estimated_time_sent);

	/**
	 * @inheritDoc
	 */
	void inPostUpdate(ZCom_Node *_node, ZCom_ConnID _from, eZCom_NodeRole _remote_role,
			zU32 _rep_bits, zU32 _event_bits, zU32 _meta_bits);

	/**
	 * @inheritDoc
	 */
	void outPreReplicateNode(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) {
	}

	/**
	 * @inheritDoc
	 */
	void outPreDereplicateNode(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) {
	}

	/**
	 * @inheritDoc
	 */
	bool outPreUpdate(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role) {
		return true;
	}

	/*! @copydoc ZCom_NodeReplicationInterceptor::outPreUpdateItem(ZCom_Node*,ZCom_ConnID,eZCom_NodeRole,ZCom_Replicator*) */
	bool outPreUpdateItem(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role,
			ZCom_Replicator *_replicator) {
		return true;
	}

	/*! @copydoc ZCom_NodeReplicationInterceptor::outPostUpdate(ZCom_Node*,ZCom_ConnID,eZCom_NodeRole,zU32,zU32,zU32) */
	void outPostUpdate(ZCom_Node *_node, ZCom_ConnID _to, eZCom_NodeRole _remote_role,
			zU32 _rep_bits, zU32 _event_bits, zU32 _meta_bits);

protected:
	/**
	 * Update the player
	 */
	void updatePlayer();
};
}

#endif //_PLAYERSETTINGSINTERCEPTOR_H
