#ifndef SCRIPTWRAPPER_H
#define SCRIPTWRAPPER_H

extern "C" {
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
}
#include "luabind/luabind.hpp"

namespace HovUni {

	/**
	*	The Script Manager. This class functions as a wrapper 
	*	around basic Lua-C++ functionality.
	*	@author Dirk Delahaye
	*/
	class ScriptWrapper {
	private:
		/**
		*	The Lua State.
		*/
		lua_State* mLuaState;
	public:
		/**
		*	Constructor.
		*/
		ScriptWrapper();

		/**
		*	Destructor.
		*/
		virtual ~ScriptWrapper();

		/**
		*	Retrieve the Lua State. 
		*	Needed for defining functions in Lua.
		*	@return	Pointer to the lua_State object.
		*/
		lua_State* getLuaState();

		/**
		*	Register a C++ function for use in Lua. In the Lua script, this
		*	function can then be called using the given name.
		*	@param	name		The function name in the script.
		*	@param	function	The C++ function to be bound. (Function pointer)
		*/
		void registerFunction( std::string name, void (*function)(int));

		/**
		*	Execute a function defined in the Lua script.
		*	TODO: This method will just execute a function with integer
		*			return value and no arguments. Maybe add more options or
		*			include template?
		*	@param	function	The function call in Lua.
		*/
		int execute(const std::string function);

		/**
		*	Load a Lua Script file.
		*	@param	filename	The location of the Lua script.
		*/
		void load(const std::string filename);

		/**
		*	"Do" a string in the Lua State.
		*	@param	theString	Do this string.
		*/
		void doString(const std::string theString);
		
	};
}

#endif //SCRIPTWRAPPER_H