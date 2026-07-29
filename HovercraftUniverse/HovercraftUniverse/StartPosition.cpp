#include "StartPosition.h"
#include <OgreStringConverter.h>

namespace HovUni {

const Ogre::String StartPosition::CATEGORY("StartPosition");

StartPosition::StartPosition(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, float processInterval):
	HovercraftUniverseEntity(name,CATEGORY,position,orientation,"",processInterval,1){
}

StartPosition::StartPosition( ZCom_BitStream* announcedata ):
	HovercraftUniverseEntity(announcedata,CATEGORY,1)
{
}


void StartPosition::load(TiXmlElement * data) throw(ParseException){
	TiXmlNode * node;

	//We are loading a Finish!
	if(strcmp(data->Value(),"StartPosition") != 0){
		THROW(ParseException, "Given XML element is not of type start position.");
	}	

	//Read number
	mPlayerNumber = 0;
	node = data->FirstChild("PlayerNumber");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mPlayerNumber = Ogre::StringConverter::parseInt(Ogre::String(element->GetText()));
		}
	}
}

StartPosition::~StartPosition(void){
}

void StartPosition::setupReplication(){
	//parent
	Entity::setupReplication();

	//mPlayerNumber
	mNode->addReplicationInt(&mPlayerNumber,		// pointer to the variable
    8,												// amount of bits(up to 255 players)
    false,											// unsigned
    ZCOM_REPFLAG_MOSTRECENT,						// always send the most recent value only
    ZCOM_REPRULE_AUTH_2_ALL							// server sends to all clients
	);
}

std::string StartPosition::getClassName(){
	return "StartPosition";
}

}
