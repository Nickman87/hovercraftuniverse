#include "Position.h"

namespace HovUni {
	Position::Position(const Ogre::String& name, const Ogre::String& fileName, int width, int height, const Hikari::Position& position, Ogre::ushort zOrder)
		: BasicOverlay(name, fileName, width, height, position, zOrder) {
	}

	void Position::setNumberOfPlayers(int players) {
		this->callFunction("setTotal", Hikari::Args(players));
	}

	void Position::setCurrentPosition(int position) {
		this->callFunction("setPosition", Hikari::Args(position));
	}
}