#include "EntityRegister.h"
#include "ChatEntity.h"

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
#include "Track.h"
#include "Portal.h"
#include "PowerupSpawn.h"
#include "ResetSpawn.h"
#include "StaticBody.h"

namespace HovUni {

EntityRegister::EntityRegister(void)
{
}

EntityRegister::~EntityRegister(void)
{
}

void EntityRegister::registerAll(NetworkIDManager& manager) {
	
	//Lobby
	manager.registerClass(Lobby::getClassName());
	manager.registerClass(PlayerSettings::getClassName(), true);
	manager.registerClass(RaceState::getClassName(), true);
	manager.registerClass(RacePlayer::getClassName(), true);

	//Entities
	manager.registerClass(Asteroid::getClassName(),true);
	manager.registerClass(CheckPoint::getClassName(),true);
	manager.registerClass(Finish::getClassName(),true);
	manager.registerClass(Hovercraft::getClassName(),true);
	manager.registerClass(Portal::getClassName(),true);
	manager.registerClass(PowerupSpawn::getClassName(),true);
	manager.registerClass(ResetSpawn::getClassName(),true);
	manager.registerClass(SpeedBoost::getClassName(),true);
	manager.registerClass(Start::getClassName(),true);
	manager.registerClass(StartPosition::getClassName(),true);
	manager.registerClass(StaticBody::getClassName(),true);
}

}
