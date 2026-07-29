#include "Portal.h"
#include <OgreStringConverter.h>
#include "Hovercraft.h"

namespace HovUni {

const Ogre::String Portal::CATEGORY("Portal");

Portal::Portal(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& ogreentity, float processInterval):
	HovercraftUniverseEntity(name,CATEGORY,position,orientation,ogreentity,processInterval,0){
}

Portal::Portal( ZCom_BitStream* announcedata ):
	HovercraftUniverseEntity(announcedata,CATEGORY,0)
{
}

void Portal::load(TiXmlElement * data) throw(ParseException){
	TiXmlNode * node;
	
	//We are loading a Portal!
	if(strcmp(data->Value(),"Portal") != 0){
		 THROW(ParseException, "Given XML element is not of type portal.");
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

	//Read Start
	node = data->FirstChild("Start");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			Ogre::String value = Ogre::String(element->GetText());
			Ogre::vector<Ogre::String>::type split = Ogre::StringUtil::split(value,",");
			
			if(split.size() != 3)
				 THROW(ParseException, "Start does not have 3 splits.");

			for(int i = 0; i < 3; i++){
				mStart[i] = Ogre::StringConverter::parseReal(split[i]);
			}			
		}
	}

	//Read End
	node = data->FirstChild("End");
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			Ogre::String value = Ogre::String(element->GetText());
			Ogre::vector<Ogre::String>::type split = Ogre::StringUtil::split(value,",");
			
			if(split.size() != 3)
				 THROW(ParseException, "End does not have three splits.");

			for(int i = 0; i < 3; i++){
				mEnd[i] = Ogre::StringConverter::parseReal(split[i]);
			}			
		}
	}

	//Read Time
	node = data->FirstChild("Time");
	mTime = 0.0;
	if(node){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if(element){
			mTime = Ogre::StringConverter::parseReal(Ogre::String(element->GetText()));
		}
	}
}

Portal::~Portal(void){
}

void Portal::onEnter ( Hovercraft * hovercraft ){
}

void Portal::onLeave( Hovercraft * hovercraft ){
}

std::string Portal::getClassName(){
	return "Portal";
}

}
