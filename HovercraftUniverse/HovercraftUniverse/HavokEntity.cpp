#include "HavokEntity.h"
#include "HavokEntityType.h"
#include "Havok.h"

#include "EntityCollisionPrevention.h"

#include <Common/Base/Types/Color/hkColor.h>
#include <Common/Visualize/hkDebugDisplay.h>

#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Utilities/CharacterControl/CharacterRigidBody/hkpCharacterRigidBodyListener.h>
#include <Physics/Collide/Shape/Convex/Capsule/hkpCapsuleShape.h>

namespace HovUni {

const hkVector4 HavokEntity::UP(0,1,0);

const hkVector4 HavokEntity::FORWARD(1,0,0);

HavokEntity::HavokEntity( hkpWorld * world ):
	mWorld(world){
}

HavokEntity::~HavokEntity() {
}
	
}