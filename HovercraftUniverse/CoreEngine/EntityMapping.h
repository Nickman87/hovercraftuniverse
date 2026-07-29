#ifndef ENTITIYMAPPING_H
#define ENTITIYMAPPING_H

#include <map>
#include "OgreString.h"

namespace HovUni {

class Config;

/**
 * This class represents the Entities.ini file but in memory
 * It is read only
 */
class EntityMapping
{
private:

	/** Name of the ini file */
	static Ogre::String mEntityIniFile;

	/** The map with all info about entities */
	std::map<Ogre::String,std::map<unsigned int,Ogre::String>> mEntityIniMap;

	/** An empty map used somewhere */
	const std::map<unsigned int,Ogre::String> mEmpty;

	bool mLoaded;

	static EntityMapping mInstance;

private:

	void addMapping( Config& config, const Ogre::String& Key );

	void load();

protected:

	/**
	 * Constructor
	 */
	EntityMapping(void);

public:

	/** Default keys for the map */
	static const Ogre::String CHARACTER; // "Character"
	static const Ogre::String HOVERCRAFT; // = "Hovercraft"
	static const Ogre::String MAPS;	// = "Maps"

	/**
	 * Set the entity ini file
	 * @param file
	 */
	static void setEntityIniFile( const Ogre::String& file );

	/**
	 * Get the instance to the EntityMapping
	 * @return instance
	 */
	static EntityMapping& getInstance();	

	~EntityMapping(void);

	/**
	 * Get the name of an entity given his type and id
	 * @param type, the type of the entity
	 * @param id, the id of the entity
	 * @return the name of the entity together with the success value
	 */
	std::pair<Ogre::String,bool> getName( const Ogre::String& type, unsigned int id ) const;

	/**
	 * Get the id of an entity given his type and name
	 * @param name, the name of the entity
	 * @param id, the id of the entity
	 * @return the id of the entity together with succes value
	 */
	std::pair<unsigned int,bool> getId( const Ogre::String& type, const Ogre::String& name ) const;

	/**
	 * Get the id-name map for a given type
	 * @param type, the type
	 * @return the map for given type
	 */
	const std::map<unsigned int,Ogre::String>& getMap(const Ogre::String& type) const;
};

}

#endif
