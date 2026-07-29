#ifndef ABSTRACTHAVOKWORLD_H
#define ABSTRACTHAVOKWORLD_H

// Math and base include
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/Memory/hkThreadMemory.h>
#include <Common/Base/Memory/Memory/Pool/hkPoolMemory.h>

// World representation
#include <Physics/Utilities/Serialize/hkpPhysicsData.h>
#include <Physics/Dynamics/World/hkpWorld.h>	
#include <Common/Serialize/Packfile/hkPackfileReader.h>

// Visual Debugger includes
#include <Common/Visualize/hkVisualDebugger.h>
#include <Physics/Utilities/VisualDebugger/hkpPhysicsContext.h>	

namespace HovUni {

/**
 * Abstract class which handles a lot of basic functionality for a multithreaded havok program
 * The world will be initialised when load successfuly returns.
 */
class AbstractHavokWorld
{
private:

	// Visual Debugger
	hkVisualDebugger* vdb;

	hkpPhysicsContext* context;

	hkArray<hkProcessContext*> * contexts;

	//The real deal
	hkJobQueue* jobQueue;

	int totalNumThreadsUsed;

	hkJobThreadPool* threadPool;

	char* stackBuffer;

	hkPoolMemory* memoryManager;

	hkThreadMemory* threadMemory;

protected:

	/**
	 * Timestep
	 */
	hkReal mTimestep;

	/**
	 * A flag that shows if a world is loaded
	 */
	bool mIsLoaded;

	/**
	 * The world used in simulation
	 */
	hkpWorld* mPhysicsWorld;

	/**
	 * The world in memory as loaded from file
	 */
	hkPackfileReader::AllocatedData* mLoadedData;
	hkpPhysicsData* mPhysicsData;

public:

	/**
	 * Constructor
	 */
	AbstractHavokWorld(hkReal mTimestep = 1.0/60.0f);

	/**
	 * Destructor
	 */
	~AbstractHavokWorld(void);

	/**
	 * Get the world timestep
	 * @return timestep
	 */
	inline const hkReal getTimeStep() const {
		return mTimestep;
	}

	/**
	 * Load a hkv file
	 * This will fill the mPhysicsWorld, mLoadedData and mPhysicsData
	 * It will also set the loaded flag
	 * @param filanem
	 */
	virtual bool load ( const char * filename );

	/**
	 * Check the loaded flag
	 * @return true if world is loaded, false if not
	 */
	inline bool isLoaded() const {
		return mIsLoaded;
	}

	/**
	 * Is called every step of the simulation before step method is called
	 */
	virtual void preStep(){};

	/**
	 * Is called every step of the simulation after step method is called
	 */
	virtual void postStep(){};
	
	/**
	 * Update the world
	 * @return false if not loaded, true otherwise
	 */
	bool step();
};

}

#endif
