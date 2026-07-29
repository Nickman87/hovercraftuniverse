#include "ResetSpawn.h"
#include <OgreStringConverter.h>

namespace HovUni {

const Ogre::String ResetSpawn::CATEGORY("ResetSpawn");

ResetSpawn::ResetSpawn(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, float processInterval):
	HovercraftUniverseEntity(name,CATEGORY,position,orientation,"TODO WHY DOES AN EMPTY  STRING CRASH OUR SYSTEM???",processInterval,0){
	mReset = 0;
}

ResetSpawn::ResetSpawn( ZCom_BitStream* announcedata ):
	HovercraftUniverseEntity(announcedata,CATEGORY,0)
{
	mReset = 0;
}

void ResetSpawn::load(TiXmlElement * data) throw(ParseException){
	TiXmlNode * node;

	//We are loading a Finish!
	if(strcmp(data->Value(),"ResetSpawn") != 0){
		THROW(ParseException, "Given XML element is not of type reset spawn.");
	}	

	mReset = 0;
	node = data->FirstChild("Checkpoint");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mReset = Ogre::StringConverter::parseInt(Ogre::String(element->GetText()));
		}
	}
}

ResetSpawn::~ResetSpawn(void){
}

std::string ResetSpawn::getClassName(){
	return "ResetSpawn";
}

}
