#include "Start.h"
#include "Hovercraft.h"
#include "RaceState.h"

namespace HovUni {

const Ogre::String Start::CATEGORY("Start");

Start::Start(const Ogre::String& name, const Ogre::Vector3& position, const Ogre::Quaternion& orientation, const Ogre::String& ogreentity, float processInterval):
	HovercraftUniverseEntity(name,CATEGORY,position,orientation,ogreentity,processInterval,0){
}

Start::Start( ZCom_BitStream* announcedata ):
	HovercraftUniverseEntity(announcedata,CATEGORY,0)
{
}

void Start::load(TiXmlElement * data){
	//We are loading a Start!
	if(strcmp(data->Value(),"Start") != 0){
		THROW(ParseException, "Given XML element is not of type start.");
	}
}

Start::~Start(void){
}

void Start::onEnter ( Hovercraft * hovercraft ){
	mRacestate->onStart(this,hovercraft->getPlayerId());
}

void Start::onLeave( Hovercraft * hovercraft ){
}

std::string Start::getClassName(){
	return "Start";
}

}
