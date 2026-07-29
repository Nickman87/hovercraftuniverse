#include "StaticBody.h"

namespace HovUni {

const Ogre::String StaticBody::CATEGORY("StaticBody");

StaticBody::StaticBody(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& ogreentity, float processInterval):
	HovercraftUniverseEntity(name,CATEGORY,position,orientation,ogreentity,processInterval,0){
}

StaticBody::StaticBody( ZCom_BitStream* announcedata ):
	HovercraftUniverseEntity(announcedata,CATEGORY,0)
{
}

StaticBody::~StaticBody(void){
}

void StaticBody::load(TiXmlElement * data) throw(ParseException){
}



void StaticBody::process(float timeSince){
}

void StaticBody::processEventsServer(ControllerEvent* cEvent){
}

void StaticBody::processEventsOwner(ControllerEvent* cEvent){
}

void StaticBody::processEventsOther(ControllerEvent* cEvent){
}

std::string StaticBody::getClassName(){
	return "StaticBody";
}


}
