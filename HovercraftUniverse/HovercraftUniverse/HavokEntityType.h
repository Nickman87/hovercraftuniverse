#ifndef HAVOKENTITYTYPE_H
#define HAVOKENTITYTYPE_H

class hkpWorldObject;

namespace HovUni {

/**
 * Class used to annotate havoc world objects with an object type.
 */
class HavokEntityType {

private:

	static int ENITYTYPEPROPERTY;	

public:
	
	enum Type {	
		NOTSET,
		PLANET,
		CHARACTER,
		STATIC
	};

	/**
	 * Get the entity type
	 * @param object
	 * @return EntityType
	 */
	static Type getEntityType( const hkpWorldObject * object );

	/**
	 * Update the entity type, this should be done only when propery already set
	 * @param object
	 * @param type
	 */
	static void updateEntityType( hkpWorldObject * object, Type type );

	/**
	 * Check if object is given type
	 * @param object
	 * @param type
	 * @return true if object is of given type, false otherwise
	 */
	static bool isEntityType( const hkpWorldObject * object, Type type );	
	
	/**
	 * Set the entity type, this should be done only once
	 * @param object
	 * @param type
	 */
	static void setEntityType( hkpWorldObject * object, Type type );
};

}

#endif 