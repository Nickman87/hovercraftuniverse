#ifndef HAVOK_H
#define HAVOK_H

#include "HoverCraftUniverseWorld.h"

namespace HovUni {

DWORD WINAPI runHavok( LPVOID lpParam );

class Havok
{
private:

	friend DWORD WINAPI runHavok( LPVOID lpParam );

	static HoverCraftUniverseWorld * ms_world;

	Havok(void);
	~Havok(void);

public:

	/**
	 * Start havok framework
	 */
	static void start( const char * filename, Loader * loader );

	/**
	 * Get a pointer to the world object
	 * @return world
	 */
	static HoverCraftUniverseWorld * getSingletonPtr();

	/**
	 * Get a reference to the world object
	 * @return world
	 */
	static HoverCraftUniverseWorld& getSingleton();

	/**
	 * Stop havok framework
	 */
	static void stop();


};

}

#endif