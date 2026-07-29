#include "Track.h"
#include <OgreStringConverter.h>

namespace HovUni {

Track::Track(){
}

void Track::load(TiXmlElement * data) throw(ParseException){
	TiXmlNode * node;

	//We are loading a track!
	if(strcmp(data->Value(),"Track") != 0){
		THROW(ParseException, "Given XML element is not of type track.");
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

	//Read MinimumPlayers
	mMinimumPlayers = 0;
	node = data->FirstChild("MinimumPlayers");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mMinimumPlayers = Ogre::StringConverter::parseInt(Ogre::String(element->GetText()));
		}
	}

	//Read MinimumPlayers
	mMaximumPlayers = 0;
	node = data->FirstChild("MaximumPlayers");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mMaximumPlayers = Ogre::StringConverter::parseInt(Ogre::String(element->GetText()));
		}
	}

	//Read Physics File Name
	mPhysicsFileName = "";
	node = data->FirstChild("PhysicsFileName");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mPhysicsFileName = Ogre::String(element->GetText());
		}
	}

	//Read Physics File Name
	mPathFileName = "";
	node = data->FirstChild("PathFileName");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mPathFileName = Ogre::String(element->GetText());
		}
	}
}

Track::~Track(void){
}

}
