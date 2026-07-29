#include "Asteroid.h"
#include "Exception.h"
#include <OgreStringConverter.h>
#include "String_Replicator.h"

namespace HovUni {

const Ogre::String Asteroid::CATEGORY("Asteroid");

Asteroid::Asteroid(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& ogreentity, float processInterval):
		HovercraftUniverseEntity(name,CATEGORY,position,orientation,ogreentity,processInterval,3) {
	// Empty
}

Asteroid::Asteroid( ZCom_BitStream* announcedata ):
	HovercraftUniverseEntity(announcedata,CATEGORY,3)
{
}

void Asteroid::load(TiXmlElement * data) {
	TiXmlNode * node;

	// We are loading a CheckPoint!
	if (strcmp(data->Value(),"Asteroid") != 0) {
		THROW(ParseException, "Parsed entity is not an asteroid.");
	}

	// Read name
	mDisplayName = "No name";
	node = data->FirstChild("Name");
	if (node) {
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if (element) {
			mDisplayName = Ogre::String(element->GetText());
		}
	}

	// Read gravity
	mGravity = 0;
	node = data->FirstChild("Gravity");
	if (node) {
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if (element) {
			mGravity = Ogre::StringConverter::parseReal(Ogre::String(element->GetText()));
		}
	}

	// Read type
	mAsteroidType = UNKNOWN;
	node = data->FirstChild("AsteroidType");
	if (node) {
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if (element) {
			mAsteroidType = (AsteroidType) Ogre::StringConverter::parseInt(Ogre::String(element->GetText()));
		}
	}
}

Asteroid::~Asteroid(void) {
	// Empty
}

void Asteroid::setupReplication(){
	//set up parent
	Entity::setupReplication();

	//mName
	mNode->addReplicator(
		new String_Replicator(&mDisplayName,
		ZCOM_REPFLAG_MOSTRECENT,
		ZCOM_REPRULE_AUTH_2_ALL
	), 
	true);

	//mGravity
	mNode->addReplicationFloat(&mGravity,
	4,
	ZCOM_REPFLAG_MOSTRECENT,
	ZCOM_REPRULE_AUTH_2_ALL
	);

	//mAsteroidtype
	mNode->addReplicationInt((int*)&mAsteroidType,	// pointer to the variable
    4,												// amount of bits(up to 16 types)
    false,											// unsigned
    ZCOM_REPFLAG_MOSTRECENT,						// always send the most recent value only
    ZCOM_REPRULE_AUTH_2_ALL							// server sends to all clients
	);
}

std::string Asteroid::getClassName(){
	return "Asteroid";
}

}
