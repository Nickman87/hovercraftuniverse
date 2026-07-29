#include "CheckPoint.h"
#include <OgreStringConverter.h>
#include "String_Replicator.h"
#include "Hovercraft.h"
#include "RaceState.h"

namespace HovUni {

const Ogre::String CheckPoint::CATEGORY("Checkpoint");

CheckPoint::CheckPoint(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation,const Ogre::String& ogreentity, float processInterval):
	HovercraftUniverseEntity(name,CATEGORY,position,orientation,ogreentity,processInterval,2){
}

CheckPoint::CheckPoint(ZCom_BitStream* announcedata):
	HovercraftUniverseEntity(announcedata,CATEGORY,2){
}

CheckPoint::~CheckPoint(void){
}

std::string CheckPoint::getClassName(){
	return "Checkpoint";
}

void CheckPoint::load(TiXmlElement * data) throw(ParseException){
	TiXmlNode * node;

	//We are loading a CheckPoint!
	if(strcmp(data->Value(),"CheckPoint") != 0){
		THROW(ParseException, "Given XML element is not of type checkpoint.");
	}	

	//Read name
	mDisplayName = "No name";
	node = data->FirstChild("Name");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mDisplayName = Ogre::String(element->GetText());
		}
	}

	//Read number
	mNumber = 0;
	node = data->FirstChild("Number");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mNumber = Ogre::StringConverter::parseInt(Ogre::String(element->GetText()));
		}
	}
}

void CheckPoint::onEnter ( Hovercraft * hovercraft ){
	mRacestate->onCheckPoint(this,hovercraft->getPlayerId());	
}


void CheckPoint::onLeave( Hovercraft * hovercraft ){
}

void CheckPoint::setupReplication(){
	Entity::setupReplication();

	//mName
	mNode->addReplicator(
		new String_Replicator(&mDisplayName,
			ZCOM_REPFLAG_MOSTRECENT,
			ZCOM_REPRULE_AUTH_2_ALL), 
		true
	);

	//mNumber
	mNode->addReplicationInt(
		&mNumber,				// pointer to the variable
		sizeof(Ogre::int32),									// amount of bits(up to 16 types)
		false,											// unsigned
		ZCOM_REPFLAG_MOSTRECENT,						// always send the most recent value only
		ZCOM_REPRULE_AUTH_2_ALL							// server sends to all clients
	);
}


}
