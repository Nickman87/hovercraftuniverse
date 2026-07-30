#include "HavokThread.h"
#include "Havok.h"
#include "CustomOgreMaxScene.h"
#include "Loader.h"
#include "DedicatedServer.h"
#include "Config.h"
#include "Exception.h"
#include <exception>

// timeBeginPeriod/timeEndPeriod: raise the Windows system timer granularity for
// the lifetime of the physics thread. Without this, Sleep() only wakes up on
// ~15.6 ms boundaries, which made the shipped "30 Hz" (Sleep(33)) actually
// overshoot to roughly 21 Hz, and makes 60/144 Hz targets unreachable via
// Sleep() alone. See docs/porting/timing-and-smoothing.md.
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")

namespace {

	struct LoadParameter {
		HovUni::Loader * loader;
		const char * file;
	};

	// How far behind the deadline schedule is allowed to drift (in multiples of
	// the physics step dt) before we give up trying to catch up and simply
	// resynchronize to "now" instead. Without this clamp, a hitch (e.g. a debugger
	// break, a GC-like stall, or a slow frame) would otherwise cause the loop to
	// run many steps back-to-back trying to catch up -- a death spiral where the
	// simulation falls further and further behind while burning 100% CPU.
	const double HU_PHYSICS_MAX_CATCHUP_STEPS = 4.0;

	// When Sleep()-ing towards the deadline, stop sleeping this many milliseconds
	// early and short-spin the remainder. This leaves margin for Sleep()'s own
	// scheduling slop (still present even at 1 ms timer granularity) so we land
	// on the deadline instead of consistently overshooting it.
	const double HU_PHYSICS_SLEEP_MARGIN_MS = 2.0;

}

namespace HovUni {

bool HavokThread::run = false;
bool HavokThread::havokdebug = false;

HANDLE HavokThread::startevent;
STARTUPINFO HavokThread::si;
PROCESS_INFORMATION HavokThread::pi;
HANDLE HavokThread::handle;


void HavokThread::StopHavokThread(){
	if ( HavokThread::run = true ){
		HavokThread::run = false;
		WaitForSingleObject(HavokThread::handle,INFINITE);
	}
}

void HavokThread::StartHavokThread( const char * filename, Loader * loader ){

    ZeroMemory( &si, sizeof(HavokThread::si) );
    HavokThread::si.cb = sizeof(HavokThread::si);
    ZeroMemory( &HavokThread::pi, sizeof(HavokThread::pi) );
	
	HavokThread::startevent = CreateEvent( 
        NULL,               // default security attributes
        TRUE,               // manual-reset event
        FALSE,              // initial state is nonsignaled
        TEXT("Havoc Start Event")  // object name
        ); 

	LoadParameter params;
	params.file = filename;
	params.loader = loader;

	HavokThread::run = true;
	HavokThread::handle = CreateThread( 
            NULL,			// default security attributes
            0,               // use default stack size  
            runHavok,       // thread function name
            &params,          // argument to thread function 
            0,              // use default creation flags 
            0);   // returns the thread identifier

	//wait untill at least the havoc world is set up
	WaitForSingleObject (HavokThread::startevent,INFINITE);

	//delete creation event
	CloseHandle(HavokThread::startevent);
}

DWORD WINAPI runHavok( LPVOID lpParam ) {
	try {
		LoadParameter * params = (LoadParameter*) lpParam;

		//THE HAVOK FRAMERATE IS NOW CONTROLLED THROUGH SCRIPT.
		//change data/engine_settings.cfg to change this!
		int fps = DedicatedServer::getEngineSettings()->getValue<int>("Havok", "Framerate", 30);
		HoverCraftUniverseWorld * world = new HoverCraftUniverseWorld(1.0f/(float) fps);

		Havok::ms_world = world;

		//load havok world should be done in THIS thread
		CustomOgreMaxScene scene;
		scene.Load(params->file,params->loader);

		//notify that the world has loaded
		SetEvent(HavokThread::startevent);

		// Physics timing loop: a QueryPerformanceCounter-based deadline
		// accumulator. This replaces the original Sleep(dt*1000)-after-step
		// approach (see docs/porting/timing-and-smoothing.md for the full
		// writeup), which had two problems: (1) default ~15.6ms Windows timer
		// granularity meant Sleep(33) actually took ~47ms, so the shipped
		// "30 Hz" ran at roughly 21 Hz; and (2) sleeping a fixed duration AFTER
		// stepping makes the real period dt + step_cost, drifting under load.
		//
		// timeBeginPeriod(1) below raises timer granularity to ~1ms for the life
		// of this thread so Sleep() is precise enough to hit 60/144 Hz targets.
		timeBeginPeriod(1);

		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);

		const double dt = (double) world->getTimeStep();
		const double dtMs = dt * 1000.0;

		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);
		double nextDeadlineMs = (double) now.QuadPart * 1000.0 / (double) frequency.QuadPart;

		while ( HavokThread::run ) {
			world->step();

			nextDeadlineMs += dtMs;

			QueryPerformanceCounter(&now);
			double nowMs = (double) now.QuadPart * 1000.0 / (double) frequency.QuadPart;

			// Catch-up clamp: if we've fallen behind by more than a few steps
			// (e.g. a stall stole a chunk of wall-clock time), resync to "now"
			// instead of trying to burn through a backlog of steps -- avoids a
			// death spiral where the loop can never catch up under load.
			if (nowMs - nextDeadlineMs > HU_PHYSICS_MAX_CATCHUP_STEPS * dtMs) {
				nextDeadlineMs = nowMs;
			}

			// Sleep for the bulk of the remaining time, leaving a small margin,
			// then short-spin (yielding the core) until the exact deadline.
			for (;;) {
				QueryPerformanceCounter(&now);
				nowMs = (double) now.QuadPart * 1000.0 / (double) frequency.QuadPart;

				double remainingMs = nextDeadlineMs - nowMs;
				if (remainingMs <= 0.0) {
					break;
				}

				if (remainingMs > HU_PHYSICS_SLEEP_MARGIN_MS) {
					Sleep( (DWORD) (remainingMs - HU_PHYSICS_SLEEP_MARGIN_MS) );
				} else {
					YieldProcessor();
				}
			}
		}

		timeEndPeriod(1);

		delete world;
	} catch (HovUni::Exception & e) {
		MessageBox(NULL, e.getMessage().c_str(), "HovUni Exception in HavokThread!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
	} catch (Ogre::Exception & e) {
		MessageBox(NULL, e.getFullDescription().c_str(), "Ogre Exception in HavokThread!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
	} catch (std::exception & e) {
		MessageBox(NULL, e.what(), "Exception in HavokThread!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
	}  catch (...) {
		MessageBox(NULL, "An unknown exception occurred.", "Error in HavokThread!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
	}
	return 0;
}

}