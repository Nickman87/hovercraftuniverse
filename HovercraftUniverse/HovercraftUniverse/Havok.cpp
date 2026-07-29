#include "Havok.h"
#include "HavokThread.h"

namespace HovUni {

HoverCraftUniverseWorld * Havok::ms_world = 0;

Havok::Havok(void)
{
}

Havok::~Havok(void)
{
}

void Havok::start( const char * filename, Loader * loader ) {
	ms_world = 0;
	HavokThread::StartHavokThread( filename, loader );
}


HoverCraftUniverseWorld * Havok::getSingletonPtr() {
	return ms_world;
}


HoverCraftUniverseWorld& Havok::getSingleton() {
	assert(ms_world);
	return *ms_world;
}

void Havok::stop() {
	HavokThread::StopHavokThread();
	ms_world = 0;
}

}
