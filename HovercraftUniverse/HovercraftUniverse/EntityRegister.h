#ifndef ENTITYREGISTER_H_
#define ENTITYREGISTER_H_

#include "NetworkIDManager.h"

namespace HovUni {

/**
 * Class that will register the entity to the network ID manager
 *
 * @author Olivier Berghmans
 */
class EntityRegister
{
public:
	/**
	 * Register a set of entity classes
	 *
	 * @param manager the current ID manager
	 */
	static void registerAll(NetworkIDManager& manager);

private:
	EntityRegister(void);
	~EntityRegister(void);
};

}

#endif
