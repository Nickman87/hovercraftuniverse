#ifndef _PLAYERSETTINGSLISTENER_H
#define _PLAYERSETTINGSLISTENER_H

#include <string>

namespace HovUni {
	/**
	 * A listener interface for player setting updates
	 *
	 * @author Nick De Frangh
	 */
	class PlayerSettingsListener {
		public:
			/**
			 * Function that gets called when a PlayerSettings object gets updated.
			 *
			 * @param id The id of the updated PlayerSettings object.
			 * @param username The new username
			 * @param character The new character name
			 * @param car The new car name
			 */
			virtual void onPlayerUpdate(int id, const std::string& username, const std::string& character, const std::string& car) = 0;
	};
}

#endif //_PLAYERSETTINGSLISTENER_H