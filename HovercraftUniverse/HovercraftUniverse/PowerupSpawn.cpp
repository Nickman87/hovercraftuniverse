#include "PowerupSpawn.h"

namespace HovUni {

const Ogre::String PowerupSpawn::CATEGORY("PowerupSpawn");

PowerupSpawn::PowerupSpawn(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& ogreentity, float processInterval):
	HovercraftUniverseEntity(name,CATEGORY,position,orientation,ogreentity,processInterval,0){
}

void PowerupSpawn::load(TiXmlElement * data) throw(ParseException){
	//We are loading a CheckPoint!
	if(strcmp(data->Value(),"PowerupSpawn") != 0){
		THROW(ParseException, "Given XML element is not of type powerup spawn.");
	}
}

PowerupSpawn::PowerupSpawn( ZCom_BitStream* announcedata ):
	HovercraftUniverseEntity(announcedata,CATEGORY,0)
{
}

PowerupSpawn::~PowerupSpawn(void){
}

std::string PowerupSpawn::getClassName(){
	return "PowerupSpawn";
}

void PowerupSpawn::onEnter ( Hovercraft * hovercraft ){

}

void PowerupSpawn::onLeave( Hovercraft * hovercraft ){

}

}
