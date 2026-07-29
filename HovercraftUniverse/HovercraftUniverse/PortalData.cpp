#include "PortalData.h"
#include <OgreStringConverter.h>

namespace HovUni {

PortalData::PortalData(TiXmlElement * data) throw (ParseException)
{
	TiXmlNode * node;
	
	//We are loading a Portal!
	if(strcmp(data->Value(),"Portal") != 0){
		 throw ParseException();
	}	

	//Read name
	mName = "No name";
	node = data->FirstChild("Name");
	if ( node ){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if ( element ){
			mName = Ogre::String(element->GetText());
		}
	}

	//Read Start
	node = data->FirstChild("Start");
	if ( node ){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if ( element ){
			Ogre::String value = Ogre::String(element->GetText());
			Ogre::vector<Ogre::String>::type split = Ogre::StringUtil::split(value,",");
			
			if ( split.size() != 3 )
				 throw ParseException();

			for ( int i = 0; i < 3; i++ ){
				mStart[i] = Ogre::StringConverter::parseReal(split[i]);
			}			
		}
	}

	//Read End
	node = data->FirstChild("End");
	if ( node ){
		TiXmlElement* element = dynamic_cast<TiXmlElement*>(node);
		if ( element ){
			Ogre::String value = Ogre::String(element->GetText());
			Ogre::vector<Ogre::String>::type split = Ogre::StringUtil::split(value,",");
			
			if ( split.size() != 3 )
				 throw ParseException();

			for ( int i = 0; i < 3; i++ ){
				mEnd[i] = Ogre::StringConverter::parseReal(split[i]);
			}			
		}
	}
}

PortalData::~PortalData(void)
{
}

}
