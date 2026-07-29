#include "UpdatePositionAction.h"
#include "StaticBody.h"

namespace HovUni {

UpdatePositionAction::UpdatePositionAction( hkpRigidBody * body, HovercraftUniverseEntity * gamebody, hkReal delta ):
	hkpUnaryAction(body), mStaticBody(gamebody), mDelta(delta) {
	m_entity->addEntityActivationListener(this);
	mPreviousPosition = body->getPosition();
	mPreviousOrientation = body->getRotation();
	update();	
}

UpdatePositionAction::~UpdatePositionAction(void){
	m_entity->removeEntityActivationListener(this);
}

void UpdatePositionAction::entityDeactivatedCallback (hkpEntity *entity){
	//set  correct position but set speed to 0
	hkpRigidBody* body = dynamic_cast<hkpRigidBody*>(m_entity);
	const hkVector4& position = body->getPosition();
	const hkQuaternion& quaternion = body->getRotation();

	mStaticBody->changePosition(Ogre::Vector3(position(0),position(1),position(2)));
	mStaticBody->changeOrientation( Ogre::Quaternion( quaternion(3), quaternion(0), quaternion(1), quaternion(2)));
	mStaticBody->changeVelocity(Ogre::Vector3(0,0,0));
} 
   
void UpdatePositionAction::entityActivatedCallback (hkpEntity *entity){
	//no shit
}

void UpdatePositionAction::update(){
	hkpRigidBody* body = dynamic_cast<hkpRigidBody*>(m_entity);
	const hkVector4& position = body->getPosition();
	const hkQuaternion& quaternion = body->getRotation();
	const hkVector4& speed = body->getLinearVelocity();

	
	mStaticBody->changePosition(Ogre::Vector3(position(0),position(1),position(2)));
	mStaticBody->changeOrientation( Ogre::Quaternion( quaternion(3), quaternion(0), quaternion(1), quaternion(2)));
	mStaticBody->changeVelocity(Ogre::Vector3(speed(0),speed(1),speed(2)));
}

void UpdatePositionAction::applyAction( const hkStepInfo& stepInfo ){
	//TODO check if realy needed
	//if ( !mPreviousPosition.equals3(position, delta) ){

	update();
}

}
