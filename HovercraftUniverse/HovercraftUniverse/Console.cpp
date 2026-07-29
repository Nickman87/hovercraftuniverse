#include "Console.h"
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>

namespace HovUni {

Console::Console(void)
{
}

Console::~Console(void)
{
}

void Console::createConsole(const char * title, int lines) throw(std::exception) {
	int hConHandle;
	long lStdHandle;
	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;


	// allocate a console for this app

	if (AllocConsole() == FALSE ){
		throw std::exception();
	}

	SetConsoleTitle(title);

	// set the screen buffer to be big enough to let us scroll text

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),	&coninfo);

	coninfo.dwSize.Y = lines;

	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),	coninfo.dwSize);

	// redirect unbuffered STDOUT to the console

	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);

	DWORD size;
	COORD c;
	c.X = 0;
	c.Y = 0;
	FillConsoleOutputAttribute(
	  (HANDLE)lStdHandle,
	  BACKGROUND_INTENSITY | BACKGROUND_BLUE	| BACKGROUND_GREEN | BACKGROUND_RED,
	  200000,
	  c,
	  &size
	);

	SetConsoleTextAttribute((HANDLE)lStdHandle, FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY | BACKGROUND_BLUE	| BACKGROUND_GREEN | BACKGROUND_RED);

	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen( hConHandle, "w" );

	*stdout = *fp;

	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console

	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);

	SetConsoleTextAttribute((HANDLE)lStdHandle, FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY | BACKGROUND_BLUE	| BACKGROUND_GREEN | BACKGROUND_RED);

	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen( hConHandle, "r" );

	*stdin = *fp;

	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console

	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);

	SetConsoleTextAttribute((HANDLE)lStdHandle, FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_INTENSITY | BACKGROUND_BLUE	| BACKGROUND_GREEN | BACKGROUND_RED);


	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen( hConHandle, "w" );

	*stderr = *fp;

	setvbuf( stderr, NULL, _IONBF, 0 );

	std::ios::sync_with_stdio();
}

void Console::destroyConsole() {
	FreeConsole();
}

}
