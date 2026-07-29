#ifndef CONSOLE_H
#define CONSOLE_H

#include "Exception.h"

namespace HovUni {

/**
 * This class can be used to create a console.
 * CIN/CER/COUT will be forwarderd to this console.
 * An application can only have one console!
 *
 * @author Pieter-Jan Pintens
 */
class Console
{
private:
	Console(void);
	~Console(void);

public:

	/**
	 * Create the console
	 *
	 * @param title the title of the console window
	 * @param lines the amount of buffered lines
	 */
	static void createConsole( const char * title, int lines = 500 ) throw(std::exception);

	/**
	 * Destroy the console
	 */
	static void destroyConsole();
};

}

#endif
