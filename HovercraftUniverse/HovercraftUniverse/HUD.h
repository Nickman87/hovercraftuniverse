#ifndef HUD_H_
#define HUD_H_

#include <OverlayContainer.h>

#include "Chat.h"
#include <ChatListener.h>
#include "Direction.h"
#include "Position.h"
#include "Speedometer.h"
#include "Timer.h"
#include <tinyxml/tinyxml.h>

namespace HovUni {

/**
 * The HUD is a 2D display that overlays the 3D scene to a player. It contains useful information
 * about the game state and/or provides controls that the player can use.
 *
 * @author Nick De Frangh
 */
class HUD : public OverlayContainer, public ChatListener {
private:
	/** The meter indicating the speed of the racer */
	Speedometer* mSpeedometer;

	/** Direction arrow */
	Direction* mDirection;

	/** Position indicator */
	Position* mPosition;

	/** Chat widget */
	Chat* mChat;

	/** Timer widget */
	Timer* mTimer;

	/** Indicates whether the HUD should be activated */
	bool mIsActivated;

	/** Boolean to check if the chat is activated */
	bool mChatIsActivated;

	std::string mHudName;

	/** Helper data structure */
	struct ComponentData {
		/** The name for the component */
		Ogre::String mName;
		/** The name of the file that has to be loaded */
		Ogre::String mFilename;
		/** The prosition to place the overlay */
		Hikari::Position mPosition;
		/** The width of the overlay */
		int mWidth;
		/** Boolean to check for percentage width */
		bool mWidthP;
		/** The height of the overlay */
		int mHeight;
		/** Boolean to check for percentage height */
		bool mHeightP;
	};

public:

	/**
	 * Constructor.
	 */
	HUD(TiXmlElement* HUDConfig, const Hikari::FlashDelegate& chatInput);

	/**
	 * Destructor.
	 */
	virtual ~HUD();

	/**
	 * Hide the direction arrow
	 */
	void hideDirection();

	/**
	 * Update the direction arrow in the HUD (if present)
	 *
	 * @param position The current position of the player
	 * @param forward The forward vector of the player
	 * @param goal The goal where the player has to go to
	 */
	void updateDirection(Ogre::Vector3& forward, Ogre::Vector3& destination, Ogre::Vector3& up);

	/**
	 * Update the speed visualisation [0 - 100]
	 *
	 * @param speed The speed value to visualise [0 - 100]
	 */
	void updateSpeed(float speed);

	/**
	 * Update the boost visualisation [0 - 100]
	 *
	 * @param boost The boost value to visualise [0 - 100]
	 */
	void updateBoost(float boost);

	/**
	 * Start the timer
	 */
	void startLapTimer();

	/**
	 * Stop the timer
	 */
	void stopLapTimer();

	/**
	 * Set the timer to a given value, make sure it is stopped, otherwise
	 * it will be overwritten
	 *
	 * @param minutes The minutes value
	 * @param seconds The seconds value
	 * @param hundreds The hundreds value
	 */
	void setLapTimer(int minutes, int seconds, int hundreds);
	
	/**
	 * Strip a string of unwanted characters
	 *
	 * @param value The string to strip
	 * @return The stripped string
	 */
	std::string stripString(const std::string& value);

	/**
	 * Add a new chat message to the lobby chat
	 *
	 * @param user The name of the user that posted the message
	 * @param line The message the user has posted
	 */
	virtual void newMessage(const std::string& user, const std::string& line);

	/**
	 * An incoming notification from the server
	 *
	 * @param notif the notification
	 */
	virtual void newNotification(const std::string& notif);

	/**
	 * Focus the chat window
	 */
	void focusChat();

	/**
	 * Defocus the chat window
	 */
	void defocusChat();

	/** Check if the chat is focused */
	bool isChatFocused() { return mChatIsActivated; }

	void setNumberOfPlayers(int players);

	void setCurrentPosition(int position);

private:
	void buildComponents(TiXmlElement* HUDConfig, std::vector<ComponentData*>& components, std::vector<ComponentData*>& percentageComponents );

	int fixPercentageSize(bool width, std::vector<ComponentData*>& components, int percentage, Hikari::Position pos);
};

}

#endif