#include "EntityMapping.h"
#include "Config.h"
#include "DedicatedServer.h"
#include <sstream>

namespace HovUni {

const Ogre::String EntityMapping::CHARACTER = "Character";
const Ogre::String EntityMapping::HOVERCRAFT = "Hovercraft";
const Ogre::String EntityMapping::MAPS = "Maps";
Ogre::String EntityMapping::mEntityIniFile;

EntityMapping EntityMapping::mInstance;

EntityMapping::EntityMapping(void)
{
	mLoaded = false;
}

void EntityMapping::setEntityIniFile( const Ogre::String& file ){
	mEntityIniFile = file;
}

EntityMapping& EntityMapping::getInstance(){
	if ( !mInstance.mLoaded ){
		mInstance.load();
	}
	return mInstance;
}

void EntityMapping::load(){
	Config c;
	c.loadFile(mEntityIniFile);
	addMapping(c,EntityMapping::CHARACTER);
	addMapping(c,EntityMapping::HOVERCRAFT);
	addMapping(c,EntityMapping::MAPS);
	mLoaded = true;
}

void EntityMapping::addMapping( Config& config, const Ogre::String& key ){
	std::map<unsigned int,Ogre::String> mapping;
	const Config::keymap& keymap = config.getKeys(key);
	for ( Config::keymap::const_iterator i = keymap.begin(); i != keymap.end(); i++ ){
		//Get KEY
		std::stringstream ss;
		unsigned int subkey;
		ss << i->first;
		ss >> subkey;

		mapping.insert(std::pair<unsigned int,Ogre::String>(subkey, i->second->GetValue()));		
	}

	mEntityIniMap.insert(std::pair<Ogre::String,std::map<unsigned int,Ogre::String>>(key,mapping));
}

EntityMapping::~EntityMapping(void)
{
}

std::pair<Ogre::String,bool> EntityMapping::getName( const Ogre::String& type, unsigned int id ) const
{
	const std::map<Ogre::String,std::map<unsigned int,Ogre::String>>::const_iterator i = mEntityIniMap.find(type);
	if ( i != mEntityIniMap.end()){
		std::map<unsigned int,Ogre::String>::const_iterator j = i->second.find(id);
		if ( j != i->second.end() ){
			return std::pair<Ogre::String,bool>(j->second,true);
		}
	}
	return std::pair<Ogre::String,bool>("",false);	
}


std::pair<unsigned int,bool> EntityMapping::getId( const Ogre::String& type, const Ogre::String& name ) const
{
	const std::map<Ogre::String,std::map<unsigned int,Ogre::String>>::const_iterator i = mEntityIniMap.find(type);
	if ( i != mEntityIniMap.end()){
		for ( std::map<unsigned int,Ogre::String>::const_iterator j = i->second.begin(); j != i->second.end(); j++ ){
			if ( j->second == name ){
				return std::pair<unsigned int,bool>(j->first,true);
			}
		}
	}

	return std::pair<unsigned int,bool>(0,false);
}

const std::map<unsigned int,Ogre::String>& EntityMapping::getMap(const Ogre::String& type) const{
	std::map<Ogre::String,std::map<unsigned int,Ogre::String>>::const_iterator i = mEntityIniMap.find(type);
	if ( i != mEntityIniMap.end()){
		return i->second;
	}
	else {
		return mEmpty;
	}
}


}