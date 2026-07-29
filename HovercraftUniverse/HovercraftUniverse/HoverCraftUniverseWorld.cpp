#include "HoverCraftUniverseWorld.h"

#include "ServerLoader.h"
#include "CustomOgreMaxScene.h"

#include "HavokEntityType.h"
#include "PhantomTrackAction.h"
#include "UpdatePositionAction.h"

#include "GameEntities.h"

#include "Hovercraft.h"
#include "HavokHovercraft.h"
#include "OgreMaxTypes.hpp"
#include "PlanetGravityPhantom.h"
#include "HavokEntityType.h"
#include "CheckpointPhantom.h"
#include "StartPhantom.h"
#include "SpeedBoostPhantom.h"
#include "FinishPhantom.h"
#include "PortalPhantom.h"
#include "PowerupPhantom.h"



#include <Physics/Collide/Filter/Group/hkpGroupFilterSetup.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Common/Base/Math/Matrix/hkTransform.h>

namespace {

hkpShape * setBox(const Ogre::Vector3& scale) {
	hkVector4 halfex;
	halfex.set(scale[0], scale[1], scale[2]);
	halfex.mul4(0.5f); //divided by 2 for half extend => 0.5
	return new hkpBoxShape(halfex);
}

void setBox(hkAabb& aabb, const Ogre::Vector3& position, const Ogre::Quaternion& rotation, const Ogre::Vector3& scale) {
	aabb.m_max.set(1, 1, 1);
	aabb.m_max.set(-1, -1, -1);

	hkVector4 t(position.x, position.y, position.z);
	hkQuaternion q(rotation.x, rotation.y, rotation.z, rotation.w);

	hkTransform tr(q, t);

	hkVector4 pos1;
	pos1.set(scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f);
	pos1.setTransformedPos(tr, pos1);

	hkVector4 pos2;
	pos2.set(scale.x * -0.5f, scale.y * -0.5f, scale.z * -0.5f);
	pos2.setTransformedPos(tr, pos2);

	if (pos1(0) > pos2(0)) {
		aabb.m_max(0) = pos1(0);
		aabb.m_min(0) = pos2(0);
	} else {
		aabb.m_max(0) = pos2(0);
		aabb.m_min(0) = pos1(0);
	}

	if (pos1(1) > pos2(1)) {
		aabb.m_max(1) = pos1(1);
		aabb.m_min(1) = pos2(1);
	} else {
		aabb.m_max(1) = pos2(1);
		aabb.m_min(1) = pos1(1);
	}

	if (pos1(2) > pos2(2)) {
		aabb.m_max(2) = pos1(2);
		aabb.m_min(2) = pos2(2);
	} else {
		aabb.m_max(2) = pos2(2);
		aabb.m_min(2) = pos1(2);
	}
}
}

namespace HovUni {

HoverCraftUniverseWorld::HoverCraftUniverseWorld(hkReal timestep):
	AbstractHavokWorld(timestep)
{
}

HoverCraftUniverseWorld::~HoverCraftUniverseWorld(void)
{
	hkStorageStringMap<HavokEntity*>::Iterator i = mCharactersMap.getIterator();
	while ( mCharactersMap.isValid(i) ){
		delete mCharactersMap.getValue(i);
		i = mCharactersMap.getNext(i);
	}

	mCharactersMap.clear();
}

void HoverCraftUniverseWorld::createStart( Start * start, OgreMax::Types::ExternalItem& externalitem ){
	mPhysicsWorld->markForWrite();

	//Create a phantom that handles this
	hkAabb aabb;
	setBox(aabb, externalitem.position, externalitem.rotation, externalitem.scale);
	StartPhantom * phantom = new StartPhantom(aabb, start);
	phantom->setUserData(reinterpret_cast<hkUlong>(start));
	mPhysicsWorld->addPhantom(phantom);
	phantom->removeReference();

	mPhysicsWorld->unmarkForWrite();
}

void HoverCraftUniverseWorld::createFinish( Finish * finish, OgreMax::Types::ExternalItem& externalitem ){
	mPhysicsWorld->markForWrite();

	//Create a phantom that handles this
	hkAabb aabb;
	setBox(aabb, externalitem.position, externalitem.rotation, externalitem.scale);
	FinishPhantom * phantom = new FinishPhantom(aabb, finish);
	phantom->setUserData(reinterpret_cast<hkUlong>(finish));
	mPhysicsWorld->addPhantom(phantom);
	phantom->removeReference();

	mPhysicsWorld->unmarkForWrite();
}

void HoverCraftUniverseWorld::createCheckpoint( CheckPoint * checkpoint, OgreMax::Types::ExternalItem& externalitem ){
	mPhysicsWorld->markForWrite();

	//Create a phantom that handles this
	hkAabb aabb;
	setBox(aabb, externalitem.position, externalitem.rotation, externalitem.scale);
	CheckpointPhantom * phantom = new CheckpointPhantom(aabb, checkpoint);
	phantom->setUserData(reinterpret_cast<hkUlong>(checkpoint));
	mPhysicsWorld->addPhantom(phantom);
	phantom->removeReference();

	mPhysicsWorld->unmarkForWrite();
}

void HoverCraftUniverseWorld::createPortal( Portal * portal, OgreMax::Types::ExternalItem& externalitem ){
	mPhysicsWorld->markForWrite();

	//Create a phantom that handles this
	hkAabb aabb;
	setBox(aabb, externalitem.position, externalitem.rotation, externalitem.scale);
	PortalPhantom * phantom = new PortalPhantom(aabb, portal);
	phantom->setUserData(reinterpret_cast<hkUlong>(portal));	

	mPhysicsWorld->addPhantom(phantom);
	phantom->removeReference();

	mPhysicsWorld->unmarkForWrite();
}

void HoverCraftUniverseWorld::createBoost( SpeedBoost * boost, OgreMax::Types::ExternalItem& externalitem ){
	mPhysicsWorld->markForWrite();

	//Create a phantom that handles this
	hkpShape * shape = setBox(externalitem.scale);

	hkQuaternion r;
	r.set(externalitem.rotation.x, externalitem.rotation.y, externalitem.rotation.z, externalitem.rotation.w);

	hkVector4 t;
	t.set(externalitem.position[0], externalitem.position[1], externalitem.position[2]);

	hkTransform tr(r, t);

	SpeedBoostPhantom * phantom = new SpeedBoostPhantom(shape, tr, boost);
	phantom->setUserData(reinterpret_cast<hkUlong>(boost));

	mPhysicsWorld->addPhantom(phantom);
	phantom->removeReference();
	shape->removeReference();

	mPhysicsWorld->unmarkForWrite();
}

void HoverCraftUniverseWorld::createAsteroid( Asteroid * asteroid, OgreMax::Types::ExternalItem& externalitem ){
	mPhysicsWorld->markForWrite();

	//Get the entity name of the asteroid
	Ogre::String entityname = asteroid->getOgreEntity();

	// Find the planet
	hkpRigidBody* planetRigidBody = mPhysicsData->findRigidBodyByName(entityname.c_str());

	if (planetRigidBody == HK_NULL) {
		THROW(ParseException, "No such name found.");
	}

	//Set that it is a planet
	HavokEntityType::setEntityType(planetRigidBody, HavokEntityType::PLANET);
	planetRigidBody->setUserData(reinterpret_cast<hkUlong>(asteroid));

	//add gravity field PULL
	{
		hkAabb currentAabb;
		setBox(currentAabb, externalitem.position, externalitem.rotation, externalitem.scale);

		// Attach a gravity phantom to the planet so it can catch objects which come close
		PlanetGravityPhantom* gravityphantom = new PlanetGravityPhantom(asteroid, planetRigidBody, currentAabb);
		mPhysicsWorld->addPhantom(gravityphantom);
		gravityphantom->removeReference();
	}

	mPhysicsWorld->unmarkForWrite();
}

void HoverCraftUniverseWorld::createStaticBody( StaticBody * entity ){
	mPhysicsWorld->markForWrite();

	//Get the entity name of the asteroid
	Ogre::String entityname = entity->getOgreEntity();
	
	// Find the body
	hkpRigidBody* body = mPhysicsData->findRigidBodyByName(entityname.c_str());

	if (body == HK_NULL) {
		THROW(ParseException, "No such name found.");
	}

	//add update action, this will propagate the position of the physic rigid body to the game entity
	mPhysicsWorld->addAction(new UpdatePositionAction(body,entity));

	mPhysicsWorld->unmarkForWrite();
}



void HoverCraftUniverseWorld::postStep() {
}

void HoverCraftUniverseWorld::preStep() {
	//update characters
	hkStorageStringMap<HavokEntity*>::Iterator i = mCharactersMap.getIterator ();
	while ( mCharactersMap.isValid(i) ){
		HavokEntity * ch = mCharactersMap.getValue(i); 
		ch->update();
		i = mCharactersMap.getNext(i);
	}
}

HavokEntity * HoverCraftUniverseWorld::getCharacter(const char * name) {
	if (name == 0 )
		return HK_NULL;

	HavokEntity * result = HK_NULL;
	mCharactersMap.get(name,&result);
	return result;
}

void HoverCraftUniverseWorld::createHovercraft( Hovercraft * entity, const hkString& hovercraft, const hkString& entityname, const hkVector4& pos, const hkQuaternion& rot ){
	HavokHovercraft * c = new HavokHovercraft(mPhysicsWorld,entity,hovercraft,entityname);
	c->load(pos,rot);
	mCharactersMap.insert(entity->getName().c_str(),c);
}

}
