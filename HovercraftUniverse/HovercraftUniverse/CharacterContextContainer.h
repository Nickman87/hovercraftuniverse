#ifndef CHARACTERCONTEXTCONTAINER_H
#define CHARACTERCONTEXTCONTAINER_H

#include <Physics/Utilities/CharacterControl/StateMachine/hkpDefaultCharacterStates.h>
#include <Common/Base/Container/StringMap/hkStorageStringMap.h>

namespace HovUni {

/**
 * Character context container
 * This is a container for all kinds of character contexts.
 * It contains a mapping of a name to a context.
 * There exists one Default context named "Default"
 */
class CharacterContextContainer {

private:
	
	typedef hkStorageStringMap<hkpCharacterContext*> CharacterContextMap;

	/**
	 * Map
	 */
	CharacterContextMap mMap;

public:

	/**
	 * Constructor
	 */
	CharacterContextContainer();

	/**
	 * Destructor
	 */
	~CharacterContextContainer();

	/**
	 * Register a context under a given name
	 */
	void registerContext( const char * key, hkpCharacterContext * context );

	/**
	 * Return the context with given name
	 * @return the context, HK_NULL if not such context exists, don't forget to remove the reference when done!
	 */
	hkpCharacterContext * getCharacterContext( const char * key );
};

}

#endif