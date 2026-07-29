#ifndef HAVOKTHREAD_H
#define HAVOKTHREAD_H

#include "windows.h"
#include "Loader.h"

namespace HovUni {

DWORD WINAPI runHavok( LPVOID lpParam );

class HavokThread {

private:

	friend DWORD WINAPI runHavok( LPVOID lpParam );

	static bool run;

	static HANDLE handle;
	static HANDLE startevent;

	static bool havokdebug;
	static STARTUPINFO si;
    static PROCESS_INFORMATION pi;

public:

	static void StartHavokThread( const char * filename, Loader * loader );

	static void StopHavokThread();

};

}

#endif