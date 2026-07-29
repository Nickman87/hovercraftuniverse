#ifndef HoverCraftUniverseWorld_H
#define HoverCraftUniverseWorld_H

#include "AbstractHavokWorld.h"
#include "ServerLoader.h"
#include "HavokEntity.h"
#include "CharacterContextContainer.h"

namespace HovUni {

/**
 * This world describes our game state from the phyiscal point of view
 */
class HoverCraftUniverseWorld : public AbstractHavokWorld
{
private:

	/**
	 * Map with characters mapped on their name
	 */
	hkStorageStringMap<HavokEntity*> mCharactersMap;

public:

	/**
	 * Constructor
	 */
	HoverCraftUniverseWorld( hkReal timestep );
	
	/**
	 * Destructor
	 */
	virtual ~HoverCraftUniverseWorld(void);

	/**
	 * Create an asteroid given the extra data 
	 * @param asteroid
	 * @param externalitem
	 */
	void createAsteroid( Asteroid * asteroid, OgreMax::Types::ExternalItem & externalitem );

	/**
	 * Create a start given the extra data 
	 * @param start
	 * @param externalitem
	 */
	void createStart( Start * start, OgreMax::Types::ExternalItem & externalitem );

	/**
	 * Create a finish given the extra data 
	 * @param finish
	 * @param externalitem
	 */
	void createFinish( Finish * finish, OgreMax::Types::ExternalItem & externalitem );

	/**
	 * Create a checkpoint given the extra data 
	 * @param checkpoint
	 * @param externalitem
	 */
	void createCheckpoint( CheckPoint * checkpoint, OgreMax::Types::ExternalItem & externalitem );

	/**
	 * Create a portal given the extra data 
	 * @param portal
	 * @param externalitem
	 */
	void createPortal( Portal * portal, OgreMax::Types::ExternalItem & externalitem );

	/**
	 * Create a boost given the extra data 
	 * @param boost
	 * @param externalitem
	 */
	void createBoost( SpeedBoost * boost, OgreMax::Types::ExternalItem & externalitem );

	/**
	 * Add a static body
	 */
	void createStaticBody( StaticBody * entity );

	/**
	 * Get a character with given name
	 * @param name
	 * @return character, HK_NULL if not found
	 */
	HavokEntity * getCharacter(const char * name);

	/**
	 * Add a hovercraft with given name on a given start position
	 * @param name
	 * @param pos
	 */
	void createHovercraft( Hovercraft * entity, const hkString& filename, const hkString& entityname, const hkVector4& pos, const hkQuaternion& rot );

	/**
	 * Pre simulation steps
	 */
	virtual void preStep();

	/**
	 * Post simulation steps
	 */
	virtual void postStep();

};

}

#endif
