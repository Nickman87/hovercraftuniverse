#include "AbstractHavokWorld.h"

/////////////////////////// Havoc License & Optimization Things ///////////////////////////////////

// Keycode
#include <Common/Base/keycode.cxx>

#if !defined USING_HAVOK_PHYSICS
#error Physics is needed to build this demo. It is included in the common package for reference only.
#endif

// Classlists
#define INCLUDE_HAVOK_PHYSICS_CLASSES
#define HK_CLASSES_FILE <Common/Serialize/Classlist/hkClasses.h>
#include <Common/Serialize/Util/hkBuiltinTypeRegistry.cxx>

// Generate a custom list to trim memory requirements
#define HK_COMPAT_FILE <Common/Compat/hkCompatVersions.h>
#include <Common/Compat/hkCompat_None.cxx>

/////////////////////////// Includes ///////////////////////////////////

#include <Common/Base/Thread/Job/ThreadPool/Cpu/hkCpuJobThreadPool.h>
#include <Common/Base/Thread/Job/ThreadPool/Spu/hkSpuJobThreadPool.h>
#include <Common/Base/Thread/JobQueue/hkJobQueue.h>

#include <Common/Base/System/Error/hkDefaultError.h>
#include <Common/Base/Monitor/hkMonitorStream.h>
#include <Common/Base/System/Io/IStream/hkIStream.h>

#include <Physics/Utilities/Serialize/hkpHavokSnapshot.h>

/////////////////////////// Error report ///////////////////////////////////

//TODO make it better??
#include <stdio.h>
static void HK_CALL errorReport(const char* msg, void*)
{
	printf("%s", msg);
}

/////////////////////////// Class ///////////////////////////////////

namespace HovUni {

AbstractHavokWorld::AbstractHavokWorld(hkReal timestep):
	mIsLoaded(false), mTimestep(timestep)
{
	//
	// Initialize the base system including our memory system
	//

	memoryManager = new hkPoolMemory();
	threadMemory = new hkThreadMemory(memoryManager);
	hkBaseSystem::init( memoryManager, threadMemory, errorReport );
	memoryManager->removeReference();

	// We now initialize the stack area to 100k (fast temporary memory to be used by the engine).
	int stackSize = 0x100000;
	stackBuffer = hkAllocate<char>( stackSize, HK_MEMORY_CLASS_BASE);
	hkThreadMemory::getInstance().setStackArea( stackBuffer, stackSize);

	// Get the number of physical threads available on the system
	hkHardwareInfo hwInfo;
	hkGetHardwareInfo(hwInfo);
	totalNumThreadsUsed = hwInfo.m_numThreads;

	// We use one less than this for our thread pool, because we must also use this thread for our simulation
	hkCpuJobThreadPoolCinfo threadPoolCinfo;
	threadPoolCinfo.m_numThreads = totalNumThreadsUsed - 1;

	// This line enables timers collection, by allocating 200 Kb per thread.  If you leave this at its default (0),
	// timer collection will not be enabled.
	threadPoolCinfo.m_timerBufferPerThreadAllocation = 200000;
	threadPool = new hkCpuJobThreadPool( threadPoolCinfo );

	//
	// Enable monitors for this thread.
	//

	// Monitors have been enabled for thread pool threads already (see above comment).
	hkMonitorStream::getInstance().resize(200000);

	//create context for the visual debugger this is use later
	contexts = new hkArray<hkProcessContext*>();	
}

AbstractHavokWorld::~AbstractHavokWorld(void)
{
	//remove visual debugger
	mPhysicsWorld->markForWrite();

	

	vdb->removeReference();
	vdb = HK_NULL;
	
	//remove context from array
	contexts->removeAt(contexts->indexOf(context));
	// Contexts are not reference counted at the base class level by the VDB as
	// they are just interfaces really. So only delete the context after you have
	// finished using the VDB.
	context->removeReference();
	context = HK_NULL;

	mPhysicsWorld->removeReference();
	mPhysicsWorld = HK_NULL;

	mPhysicsData->removeReference();
	mPhysicsData = HK_NULL;

	mLoadedData->disableDestructors();
	mLoadedData->callDestructors();
	mLoadedData->removeReference();

	//delete the queue
	delete jobQueue;
	jobQueue = HK_NULL;

	//
	// Clean up the thread pool
	//
	threadPool->removeReference();

	// Deallocate stack area
	threadMemory->setStackArea(0, 0);
	hkDeallocate(stackBuffer);

	threadMemory->removeReference();

	delete contexts;

	// Quit base system
	hkBaseSystem::quit();
}

bool AbstractHavokWorld::load ( const char * filename ){
	//unload if a world is present
	if ( mIsLoaded )
		return true;

	// We also need to create a Job queue. This job queue will be used by all Havok modules to run multithreaded work.
	// Here we only use it for physics.
	hkJobQueueCinfo info;
	info.m_jobQueueHwSetup.m_numCpuThreads = totalNumThreadsUsed;
	jobQueue = new hkJobQueue(info);

	hkIstream infile( filename );
	if ( !infile.isOk() )
		return false;
	
	HK_ASSERT( 0x215d080c, infile.isOk() );

	mPhysicsData = hkpHavokSnapshot::load( infile.getStreamReader(), &mLoadedData );
	if ( mPhysicsData == HK_NULL ){
		return false;
	}

	HK_ASSERT( 0, mPhysicsData != HK_NULL );

	//Make Multi Threaded
	hkpWorldCinfo worldInfo;

	// Set the simulation type of the world to multi-threaded.
	worldInfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_MULTITHREADED;
	worldInfo.m_processActionsInSingleThread = true;

	worldInfo.m_broadPhaseWorldAabb.m_max = hkVector4(10000,10000,10000);
	worldInfo.m_broadPhaseWorldAabb.m_min = hkVector4(-10000,-10000,-10000);


	// Flag objects that fall "out of the world" to be automatically removed - just necessary for this physics scene
	worldInfo.m_broadPhaseBorderBehaviour = hkpWorldCinfo::BROADPHASE_BORDER_REMOVE_ENTITY;

	mPhysicsData->setWorldCinfo(&worldInfo);

	// Ensure non-multithreaded simulation for non-multithreaded platforms

	mPhysicsWorld = mPhysicsData->createWorld();

	mPhysicsWorld->markForWrite();

	// We need to register all modules we will be running multi-threaded with the job queue
	mPhysicsWorld->registerWithJobQueue( jobQueue );

	{
		context = new hkpPhysicsContext();
		hkpPhysicsContext::registerAllPhysicsProcesses(); // all the physics viewers
		context->addWorld(mPhysicsWorld); // add the physics world so the viewers can see it
		contexts->pushBack(context);
	}

	// Now we have finished modifying the world, release our write marker.
	mPhysicsWorld->unmarkForWrite();

	vdb = new hkVisualDebugger(*contexts);
	vdb->serve();

	mIsLoaded = true;

	return true;
}

bool AbstractHavokWorld::step () {

	if ( !mIsLoaded )
		return false;

	preStep();

	// <PHYSICS-ONLY>:
	// Step the physics world. This single call steps using this thread and all threads
	// in the threadPool. For other products you add jobs, call process all jobs and wait for completion.
	// See the multithreading chapter in the user guide for details
	mPhysicsWorld->stepMultithreaded( jobQueue, threadPool, mTimestep );

	postStep();

	// Step the visual debugger. We first synchronize the timer data
	context->syncTimers( threadPool );
	vdb->step();

	// Clear accumulated timer data in this thread and all slave threads
	hkMonitorStream::getInstance().reset();
	threadPool->clearTimerData();

	return true;
}

}
