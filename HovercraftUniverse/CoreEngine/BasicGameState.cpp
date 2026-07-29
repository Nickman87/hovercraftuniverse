#include "BasicGameState.h"

namespace HovUni {
	BasicGameState::BasicGameState() {
		mGUIManager = GUIManager::getSingletonPtr();
		mSoundManager = SoundManager::getSingletonPtr();
		mInputManager = InputManager::getSingletonPtr();
	}

	void BasicGameState::onActivate() {
		
	}

	void BasicGameState::onDisable() {
		
	}

	void BasicGameState::setManager(GameStateManager* manager) {
		mManager = manager;
	}

	void BasicGameState::switchGameState(GameStateManager::GameState state) {
		mManager->switchState(state);
	}
}