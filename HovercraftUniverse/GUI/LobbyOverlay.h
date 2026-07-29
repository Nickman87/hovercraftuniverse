#ifndef _LOBBYOVERLAY_H
#define _LOBBYOVERLAY_H

#include "BasicOverlay.h"

namespace HovUni {
	/**
	 * This overlay represents a lobby object.
	 * This includes a user overview, a chat panel and basic game settings.
	 *
	 * @author Nick De Frangh
	 */
	class LobbyOverlay : public BasicOverlay {
		public:
			/**
			 * Basic constructor, see BasicOverlay for info about the parameters
			 * @param chatInput The callback function that will be called when someone
			 *			posts a new chat message
			 * @param botsValue The function that will be called when the admin toggles the checkbox for bots
			 */
			LobbyOverlay(const Hikari::FlashDelegate& hovercraftChange, const Hikari::FlashDelegate& mapChange, const Hikari::FlashDelegate& chatInput, const Hikari::FlashDelegate& botsValue, const Hikari::FlashDelegate& playerMax, const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder = 0);

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
			 * Add a new user to the userlist. This list will be sorted in order of
			 * calls to this function.
			 *
			 * @param id A unique id for this user
			 * @param username The username of the user
			 * @param character The name of the character of the user
			 * @param car The name of the var of the user
			 */
			void addUser(int id, const std::string& username, const std::string& character, const std::string& car);

			/**
			 * Edit a given user's data, identified by the id.
			 *
			 * @param id A unique id for this user
			 * @param username The username of the user
			 * @param character The name of the character of the user
			 * @param car The name of the var of the user
			 */
			void editUser(int id, const std::string& username, const std::string& character, const std::string& car);

			/**
			 * Delete the user from the list with the given ID.
			 *
			 * @param id The id of the user you want to delete
			 */
			void deleteUser(int id);

			/**
			 * Mark us as admin or client.
			 *
			 * @param admin True when you want to display admin settings, false if you
			 *		want the client display.
			 */
			void setAdmin(bool admin);

			/**
			 * Mark a certain is as admin.
			 *
			 * @param id The id of the user you want to mark as admin
			 */
			void markAdmin(int id);

			/**
			 * Set the fill with bots value.
			 *
			 * @param fill True when you want to fill with bots, false otherwise
			 */
			void setFillBots(bool fill);

			/**
			 * Set the current player maximum to a certain value.
			 *
			 * @param value The value to set into the playermax field
			 */
			void setPlayerMax(int value);

			/**
			 * This will be called after the overlay has been activated.
			 * It will mark this overlay for keyboard input
			 */
			void customActionAfterActivate();

			/**
			 * Clear the entire list of maps
			 */
			void clearMaps();
			
			/**
			 * Add a map to the dropdown box
			 * @param id The if of the map
			 * @param name The name of the map
			 */
			void addMap(int id, const std::string& name);

			/**
			 * Set a certain map as active
			 * @param id The if of the map
			 * @param name The name of the map
			 */
			void setMap(int id, const std::string& name);

			/**
			 * Clear the entire list of hovercrafts
			 */
			void clearHovercrafts();

			/**
			 * Add a hovercraft to the dropdown box
			 * @param id The if of the hovercraft
			 * @param name The name of the hovercraft
			 */
			void addHovercraft(int id, const std::string& name);

			/**
			 * Set a certain hovercraft as active
			 * @param id The if of the hovercraft
			 * @param name The name of the hovercraft
			 */
			void setHovercraft(int id, const std::string& name);
	};
}

#endif //_LOBBYOVERLAY_H