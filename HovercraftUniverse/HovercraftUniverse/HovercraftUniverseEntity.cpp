#include "HovercraftUniverseEntity.h"
#include "RaceState.h"

namespace HovUni {

HovercraftUniverseEntity::HovercraftUniverseEntity(const Ogre::String& name, const Ogre::String& category,
		const Ogre::Vector3& position, const Ogre::Vector3& orientation, const Ogre::Vector3& upvector,
		const Ogre::String& mOgreEntity, float processInterval, unsigned short replicators) :
	Entity(name, category, position, orientation, upvector, mOgreEntity, processInterval, replicators), mRacestate(0) {
}

HovercraftUniverseEntity::HovercraftUniverseEntity(const Ogre::String& name, const Ogre::String& category,
		const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& mOgreEntity,
		float processInterval, unsigned short replicators) :
	Entity(name, category, position, orientation, mOgreEntity, processInterval, replicators), mRacestate(0) {
}

HovercraftUniverseEntity::HovercraftUniverseEntity(ZCom_BitStream* announcementdata, const Ogre::String& category,
		unsigned short replicators) :
	Entity(announcementdata, category, replicators), mRacestate(0) {
}

HovercraftUniverseEntity::~HovercraftUniverseEntity() {
}

void HovercraftUniverseEntity::setRaceState(RaceState* racestate) {
	mRacestate = racestate;
}

RaceState * HovercraftUniverseEntity::getRaceState() {
	return mRacestate;
}

}
