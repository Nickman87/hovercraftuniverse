#ifndef OGRELUABINDINGS_H_
#define OGRELUABINDINGS_H_

extern "C" {
  #include "lua.h"
  #include "lualib.h"
  #include "lauxlib.h"
}
#include "luabind/luabind.hpp"
#include <OgreRoot.h>
#include <OgreEntity.h>

namespace HovUni {
	/**
	*	Bindings that add the following Ogre classes to the given Lua State	:
	*		<ul>	
	*			<li>Ogre::Vector3</li>
	*			<li>Ogre::ColourValue</li>
	*			<li>Ogre::Entity</li>
	*			<li>Ogre::SceneNode</li>
	*			<li>Ogre::Camera</li>
	*		</ul>
	*	Source: http://www.codeproject.com/KB/graphics/luabindLuaAndOgre3d.aspx
	*	@author Dirk Delahaye, Nigel Atkinson
	*/
	class OgreLuaBindings {
	private:
		/** The Lua State */
		lua_State* mLuaState;

	public:
		/** 
		*	Constructor.
		*	@param	L	The Lua State
		*/
		OgreLuaBindings(lua_State* L);

		/**
		*	Destructor.
		*/
		~OgreLuaBindings();

		/**
		*	Add class binding for Ogre::Vector3 to the Lua State.
		*/
		void OgreLuaBindings::bindVector3();

		/**
		*	Add class binding for Ogre::Vector3 to the Lua State.
		*/
		void OgreLuaBindings::bindVector4();

		/**
		*	Add class binding for Ogre::ColourValue to the Lua State.
		*/
		void OgreLuaBindings::bindColourValue();

		/**
		*	Add class binding for Ogre::Entity to the Lua State.
		*/
		void OgreLuaBindings::bindEntity();

		/**
		*	Add class binding for Ogre::SceneNode to the Lua State.
		*/
		void OgreLuaBindings::bindSceneNode();

		/**
		*	Add class binding for Ogre::Camera to the Lua State.
		*/
		void OgreLuaBindings::bindCamera();

		/**
		*	Convenience function that adds all the other class bindings in this class.
		*/
		void OgreLuaBindings::bindLua();
	};
}

#endif //OGRELUABINDINGS_H_