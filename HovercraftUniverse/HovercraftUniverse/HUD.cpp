#include "HUD.h"

#include <algorithm>
#include "GUIManager.h"
#include "StringUtils.h"
#include <map>
#include <OgreRoot.h>
#include <sstream>
#include <vector>
#include <string>

/** Helper function to check string end */
bool endsWith(const std::string& a, const std::string& b) {
    if (b.size() > a.size()) return false;
    return std::equal(a.begin() + a.size() - b.size(), a.end(), b.begin());
}

/** Helper function to convert string to integer */
int str2int (const std::string &str) {
	std::stringstream ss(str);
	int num;
	if((ss >> num).fail()) { 
		//ERROR 
	}
	return num;
}

/** Helper function to check equality of two positions */
bool equals(Hikari::Position pos1, Hikari::Position pos2) {
	if ( !(pos1.usingRelative == pos2.usingRelative) ) {
		return false;
	}

	if (pos1.usingRelative && pos2.usingRelative) {
		return (pos1.data.rel.position == pos2.data.rel.position);
	} else {
		return ( (pos1.data.abs.left == pos2.data.abs.left) && (pos1.data.abs.top == pos2.data.abs.top) );
	}
}

namespace HovUni {

	HUD::HUD(TiXmlElement* HUDConfig, const Hikari::FlashDelegate& chatInput) 
			: mIsActivated(false), mSpeedometer(0), mDirection(0), mPosition(0), mChat(0), mTimer(0), mChatIsActivated(false) {
		std::vector<ComponentData*> components;
		std::vector<ComponentData*> percentageComponents;

		//Parse the config for all components
		buildComponents(HUDConfig, components, percentageComponents);
	
		//Fix the percentage components
		for (unsigned int i = 0; i < percentageComponents.size(); ++i) {
			ComponentData* currComponent = percentageComponents[i];

			//Check width
			if ( (*currComponent).mWidthP ) {
				//Check for other components that could intervine with this component
				(*currComponent).mWidth = fixPercentageSize(true, components, (*currComponent).mWidth, (*currComponent).mPosition);
				(*currComponent).mWidthP = false;
			}

			//Check height
			if ( (*currComponent).mHeightP ) {
				//Check for other components that could intervine with this component
				(*currComponent).mHeight = fixPercentageSize(false, components, (*currComponent).mHeight, (*currComponent).mPosition);
				(*currComponent).mHeightP = false;
			}
		}

		for (unsigned int i = 0; i < components.size(); ++i) {
			ComponentData* data = components[i];

			//Initialise all the objects
			if (!strcmp(data->mName.c_str(), "speedometer")) {
				mSpeedometer = new Speedometer(data->mName, data->mFilename, data->mWidth, data->mHeight, data->mPosition);
				mSpeedometer->setBParameter(BasicOverlay::ALPHAHACK, true);
				this->addOverlay(data->mName, mSpeedometer);
				mSpeedometer->ignoreInputs(true);
			} else if (!strcmp(data->mName.c_str(), "position")) {
				mPosition = new Position(data->mName, data->mFilename, data->mWidth, data->mHeight, data->mPosition);
				mPosition->setBParameter(BasicOverlay::ALPHAHACK, true);
				this->addOverlay(data->mName, mPosition);
				mPosition->ignoreInputs(true);
			} else if (!strcmp(data->mName.c_str(), "positionBar")) {

			} else if (!strcmp(data->mName.c_str(), "timer")) {
				mTimer = new Timer(data->mName, data->mFilename, data->mWidth, data->mHeight, data->mPosition);
				mTimer->setBParameter(BasicOverlay::ALPHAHACK, true);
				this->addOverlay(data->mName, mTimer);
				mTimer->ignoreInputs(true);
			} else if (!strcmp(data->mName.c_str(), "direction")) {
				mDirection = new Direction(data->mName, data->mFilename, data->mWidth, data->mHeight, data->mPosition);
				mDirection->setBParameter(BasicOverlay::ALPHAHACK, true);
				mHudName = data->mName;
				this->addOverlay(data->mName, mDirection);
				mDirection->ignoreInputs(true);
			} else if (!strcmp(data->mName.c_str(), "chat")) {
				mChat = new Chat(chatInput, data->mName, data->mFilename, data->mWidth, data->mHeight, data->mPosition);
				mChat->setBParameter(BasicOverlay::ALPHAHACK, true);
				this->addOverlay(data->mName, mChat);
			} else if (!strcmp(data->mName.c_str(), "timeBehind")) {

			} else {
				//Unknown
				Ogre::LogManager::getSingleton().getDefaultLog()->stream() << "Warning: Unknown HUD element type \"" << data->mName << "\"";
			}
		}

		//cleanup
		for (std::vector<ComponentData*>::iterator it = components.begin(); it != components.end(); ++it) {
			delete (*it);
		}
	}

	HUD::~HUD() {
		if (mSpeedometer) {
			delete mSpeedometer;
		}

		if (mDirection) {
			delete mDirection;
		}
	}

	void HUD::updateDirection(Ogre::Vector3& forward, Ogre::Vector3& destination, Ogre::Vector3& up) {
		//Calculate angle
		if ((mDirection != 0) && (mDirection->isActivated())) {
			float angle = forward.angleBetween(destination).valueDegrees();

			float side = (forward.crossProduct(up)).dotProduct(destination);
			side /= Ogre::Math::Abs(side);

			mDirection->setAngle(angle * side);
		}
	}

	void HUD::hideDirection() {
		//Calculate angle
		if (mDirection != 0) {
			this->removeOverlay(mHudName);
		}
	}

	void HUD::updateSpeed(float speed) {
		if (mSpeedometer) {
			mSpeedometer->setSpeed(speed);
		}
	}

	void HUD::updateBoost(float boost) {
		if (mSpeedometer) {
			mSpeedometer->setBoost(boost);
		}
	}

	void HUD::startLapTimer() {
		if (mTimer) {
			mTimer->start();
		}
	}

	void HUD::stopLapTimer() {
		if (mTimer) {
			mTimer->stop();
		}
	}

	void HUD::setLapTimer(int minutes, int seconds, int hundreds) {
		if (mTimer) {
			mTimer->setTime(minutes, seconds, hundreds);
		}
	}

	std::string HUD::stripString(const std::string& value) {
		return StringUtils::htmlSpecialChars(value);
	}

	void HUD::newMessage(const std::string& user, const std::string& line) {
		if (mChat) {
			mChat->addText( stripString(user), stripString(line) );
		}
	}

	void HUD::newNotification(const std::string& notif) {
		if (mChat) {
			mChat->addAction( stripString(notif) );
		}
	}

	void HUD::focusChat() {
		Ogre::LogManager::getSingleton().getDefaultLog()->stream()
					<< "[HUD]: Focussing chat";

		if (mChat) {
			mChatIsActivated = true;
			mChat->focusChat();
		}
	}

	void HUD::defocusChat() {
		if (mChat) {
			mChatIsActivated = false;
			mChat->defocusChat();
		}
	}

	void HUD::setNumberOfPlayers(int players) {
		if (mPosition) {
			mPosition->setNumberOfPlayers(players);
		}
	}

	void HUD::setCurrentPosition(int position) {
		if (mPosition) {
			mPosition->setCurrentPosition(position);
		}
	}

	void HUD::buildComponents(TiXmlElement* HUDConfig, std::vector<ComponentData*>& components, std::vector<ComponentData*>& percentageComponents ) {
		//Build up mapping for relative positioning
		std::map<Ogre::String, Hikari::Position> relativePositions;
		relativePositions["Left"] = Hikari::Position(Hikari::Left);
		relativePositions["TopLeft"] = Hikari::Position(Hikari::TopLeft);
		relativePositions["TopCenter"] = Hikari::Position(Hikari::TopCenter);
		relativePositions["TopRight"] = Hikari::Position(Hikari::TopRight);
		relativePositions["Right"] = Hikari::Position(Hikari::Right);
		relativePositions["BottomRight"] = Hikari::Position(Hikari::BottomRight);
		relativePositions["BottomCenter"] = Hikari::Position(Hikari::BottomCenter);
		relativePositions["BottomLeft"] = Hikari::Position(Hikari::BottomLeft);
		relativePositions["Center"] = Hikari::Position(Hikari::Center);
		
		int minResWidth, minResHeight, maxResWidth, maxResHeight;
		minResWidth = atoi(HUDConfig->Attribute("min_res_width"));
		minResHeight = atoi(HUDConfig->Attribute("min_res_height"));
		maxResWidth = atoi(HUDConfig->Attribute("max_res_width"));
		maxResHeight = atoi(HUDConfig->Attribute("max_res_height"));

		int screenWidth = GUIManager::getSingletonPtr()->getResolutionWidth();
		int screenHeight = GUIManager::getSingletonPtr()->getResolutionHeight();

		float scale;
		{
			if ( (maxResWidth <= screenWidth) && (maxResHeight <= screenHeight) ) {
				scale = 1;
			} else if ( (minResWidth >= screenWidth) && (minResHeight >= screenHeight) ) {
				scale = 0;
			} else {
				float scaleX = ((float)(screenWidth - minResWidth) / (float)(maxResWidth - minResWidth));
				float scaleY = ((float)(screenHeight - minResHeight ) / (float)(maxResHeight  - minResHeight ));

				scale = (scaleX < scaleY) ? scaleX : scaleY;
			}
		}

		//Construct all HUD components
		for(TiXmlElement* elem = HUDConfig->FirstChildElement("element"); elem; elem = elem->NextSiblingElement()) {
			Ogre::String type = elem->Attribute("type");
			Ogre::String filename = elem->Attribute("file");

			bool widthPercentage = false;
			bool heightPercentage = false;

			int minWidth, minHeight, maxWidth, maxHeight, width, height;
			Hikari::Position position;

			//Handle sizes
			TiXmlElement* sizeElem = elem->FirstChildElement("size");
			TiXmlElement* minSize = sizeElem->FirstChildElement("min");
			TiXmlElement* maxSize = sizeElem->FirstChildElement("max");

			if (minSize) {
				minWidth = str2int(minSize->Attribute("width"));
				minHeight = str2int(minSize->Attribute("height"));

				if ( endsWith(minSize->Attribute("width"), "%") ) {
					widthPercentage = true;
					width = minWidth;
				}

				if ( endsWith(minSize->Attribute("height"), "%") ) {
					heightPercentage = true;
					height = minHeight;
				}
			} else {
				minWidth = minHeight = 20;
			}

			if (maxSize) {
				maxWidth = str2int(maxSize->Attribute("width"));
				maxHeight = str2int(maxSize->Attribute("height"));

				if ( endsWith(maxSize->Attribute("width"), "%") ) {
					widthPercentage = true;
					width = maxWidth;
				}
				
				if ( endsWith(maxSize->Attribute("height"), "%") ) {
					heightPercentage = true;
					height = maxHeight;
				}
			} else {
				maxWidth = maxHeight = 300;
			}

			//Calculate real sizes
			if (!widthPercentage) {	//Otherwise, we already have a width
				width = (int) (minWidth + scale * (maxWidth - minWidth));
			}
			if (!heightPercentage) { //Otherwise, we already have a height
				height = (int) (minHeight + scale * (maxHeight - minHeight));
			}
			
			//Handle position
			TiXmlElement* posElem = elem->FirstChildElement("position")->FirstChildElement("relative");
			if (posElem) {
				position = relativePositions[posElem->Attribute("val")];
			} else {
				//We have absolute positioning
				posElem = elem->FirstChildElement("position")->FirstChildElement("absolute");
				//TODO: Implement :)
			}

			//Save the object
			ComponentData* object = new ComponentData();
			object->mName = type;
			object->mFilename = filename;
			object->mPosition = position;
			object->mWidth = width;
			object->mWidthP = widthPercentage;
			object->mHeight = height;
			object->mHeightP = heightPercentage;
			components.push_back(object);
			if (widthPercentage || heightPercentage) {
				percentageComponents.push_back(object);
			}
		}
	}

	int HUD::fixPercentageSize(bool width, std::vector<ComponentData*>& components, int percentage, Hikari::Position pos) {
		if (percentage > 100) {
			percentage = 100;
		}

		int padding = 15;
		float perc = percentage / 100.0f;
		
		if (width) {
			int totalWidth = GUIManager::getSingletonPtr()->getResolutionWidth();
			int objectwidth = 0;

			if ( equals(pos, Hikari::BottomCenter) || equals(pos, Hikari::BottomLeft) || equals(pos, Hikari::BottomRight) ) {
				//Find objects in the same row
				for (unsigned int i = 0; i < components.size(); ++i) {
					ComponentData* currComp = components[i];

					if (!currComp->mWidthP) {
						if ( equals(currComp->mPosition, Hikari::BottomCenter) || equals(currComp->mPosition, Hikari::BottomLeft) || equals(currComp->mPosition, Hikari::BottomRight) ) {
							objectwidth += currComp->mWidth + padding;
						}
					}
				}

				//Return the new width
				return (int) perc * (totalWidth - objectwidth);
			} else if ( equals(pos, Hikari::Center) || equals(pos, Hikari::Left) || equals(pos, Hikari::Right) ) {
				//Find objects in the same row
				for (unsigned int i = 0; i < components.size(); ++i) {
					ComponentData* currComp = components[i];

					if (!currComp->mWidthP) {
						if ( equals(currComp->mPosition, Hikari::Center) || equals(currComp->mPosition, Hikari::Left) || equals(currComp->mPosition, Hikari::Right) ) {
							objectwidth += currComp->mWidth + padding;
						}
					}
				}

				//Return the new width
				return (int) perc * (totalWidth - objectwidth);
			} else if ( equals(pos, Hikari::TopCenter) || equals(pos, Hikari::TopLeft) || equals(pos, Hikari::TopRight) ) {
				//Find objects in the same row
				for (unsigned int i = 0; i < components.size(); ++i) {
					ComponentData* currComp = components[i];

					if (!currComp->mWidthP) {
						if ( equals(currComp->mPosition, Hikari::TopCenter) || equals(currComp->mPosition, Hikari::TopLeft) || equals(currComp->mPosition, Hikari::TopRight) ) {
							objectwidth += currComp->mWidth + padding;
						}
					}
				}

				//Return the new width
				return (int) perc * (totalWidth - objectwidth);
			}
		} else {
			int totalHeight = GUIManager::getSingletonPtr()->getResolutionHeight();
			int objectheight = 0;

			if ( equals(pos, Hikari::Left) || equals(pos, Hikari::TopLeft) || equals(pos, Hikari::BottomLeft) ) {
				//Find objects in the same row
				for (unsigned int i = 0; i < components.size(); ++i) {
					ComponentData* currComp = components[i];

					if (!currComp->mHeightP) {
						if ( equals(currComp->mPosition, Hikari::Left) || equals(currComp->mPosition, Hikari::TopLeft) || equals(currComp->mPosition, Hikari::BottomLeft) ) {
							objectheight += currComp->mHeight + padding;
						}
					}
				}

				//Return the new width
				return (int) perc * (totalHeight - objectheight);
			} else if ( equals(pos, Hikari::TopCenter) || equals(pos, Hikari::Center) || equals(pos, Hikari::BottomCenter) ) {
				//Find objects in the same row
				for (unsigned int i = 0; i < components.size(); ++i) {
					ComponentData* currComp = components[i];

					if (!currComp->mHeightP) {
						if ( equals(currComp->mPosition, Hikari::TopCenter) || equals(currComp->mPosition, Hikari::Center) || equals(currComp->mPosition, Hikari::BottomCenter) ) {
							objectheight += currComp->mHeight + padding;
						}
					}
				}

				//Return the new width
				return (int) perc * (totalHeight - objectheight);
			} else if ( equals(pos, Hikari::TopRight) || equals(pos, Hikari::Right) || equals(pos, Hikari::BottomRight) ) {
				//Find objects in the same row
				for (unsigned int i = 0; i < components.size(); ++i) {
					ComponentData* currComp = components[i];

					if (!currComp->mHeightP) {
						if ( equals(currComp->mPosition, Hikari::TopRight) || equals(currComp->mPosition, Hikari::Right) || equals(currComp->mPosition, Hikari::BottomRight) ) {
							objectheight += currComp->mHeight + padding;
						}
					}
				}

				//Return the new width
				return (int) perc * (totalHeight - objectheight);
			}
		}

		//Should never occure...
		Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "Warning: Unknown position with percentage was given";
		return 0;
	}
}

