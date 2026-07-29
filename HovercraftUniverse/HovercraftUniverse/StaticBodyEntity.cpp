#include "StaticBodyEntity.h"
#include "StaticBody.h"
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

namespace HovUni {

StaticBodyEntity::StaticBodyEntity(hkpWorld * world, hkpRigidBody * Body, StaticBody * staticBody):
	HavokEntity(world), mBody(Body), mStaticBody(staticBody){
}

StaticBodyEntity::~StaticBodyEntity(void){
}

const hkVector4& StaticBodyEntity::getUp() const{
	return mUp;
}

const hkVector4& StaticBodyEntity::getPosition() const{
	return mBody->getPosition();	
}

const hkVector4& StaticBodyEntity::getVelocity() const{
	return mBody->getLinearVelocity();
}

const hkQuaternion& StaticBodyEntity::getOrientation() const{
	return mBody->getRotation();
}

hkpRigidBody * StaticBodyEntity::getRigidBody() const{
	return mBody;
}

Entity * StaticBodyEntity::getEntity(){
	return mStaticBody;
}

void StaticBodyEntity::update(){
}

void StaticBodyEntity::updateUp( const hkVector4& newUp){
	mUp = newUp;
}

}