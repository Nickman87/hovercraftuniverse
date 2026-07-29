#ifndef _CHAT_H
#define _CHAT_H

#include "BasicOverlay.h"

namespace HovUni {
	/**
	 * An overlay containing a chat representation.
	 *
	 * @author Nick De Frangh
	 */
	class Chat : public BasicOverlay {
		public:
			/**
			 * Chat constructor, see BasicOverlay for parameter description
			 */
			Chat(const Hikari::FlashDelegate& chatInput, const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder = 0);

			/**
			 * Add a new chat message to the lobby chat
			 *
			 * @param username The name of the user that posted the message
			 * @param message The message the user has posted
			 */
			void addText(const Ogre::String& username, const Ogre::String& message);

			/**
			 * Add a new action to the lobby chat
			 *
			 * @param message The message the action should display
			 */
			void addAction(const Ogre::String& message);

			/**
			 * Call this function to automaticly focus the chat window so the user
			 * can start typing.
			 */
			void focusChat();

			/**
			 * Call this function to remove all focus from the window.
			 */
			void defocusChat();
	};
}

#endif //_CHAT_H