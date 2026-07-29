#include "HUApplication.h"
#include "MainMenuState.h"
#include "ControlsReader.h"
#include <HovSound.h>
#include "ProgressMonitor.h"

#include <OgreLogManager.h>

namespace HovUni {

ClientPreparationLoader * HUApplication::msPreparationLoader = 0;

HUApplication::HUApplication(const std::string& configINI) :
		Application("Hovercraft Universe", configINI) {
	// Register listener
	ProgressMonitor::getSingletonPtr()->addListener(this);
}

HUApplication::~HUApplication() {
	// Empty
}

void HUApplication::playMusic(SoundManager * soundMgr) {
	// Play normal background music
	//soundMgr->startAmbient(MUSICCUE_HOVSOUND_BACKGROUND_NORMAL);
	soundMgr->updateListenerPosition(new Ogre::Vector3(-10.0f, 40.0f, 0.0f));
}

BasicGameState * HUApplication::getInitialGameState() {
	return new MainMenuState();
}

void HUApplication::customSceneSetup() {
	// Initialize the client preparation loader
	Ogre::LogManager::getSingleton().getDefaultLog()->stream()
			<< "[HUApplication]: Creating client preparation loader";
	msPreparationLoader = new ClientPreparationLoader();
}

void HUApplication::init() {
	Application::init();

	// read controls
	ControlsReader * controls = new ControlsReader(mInputManager->getKeyManager());
	controls->readControls();
}

void HUApplication::updateProgress(double progress) {
	// TODO Put this in the GUI where it belongs
	Ogre::LogManager::getSingletonPtr()->getDefaultLog()->stream() << "PROGRESS UPDATE " << progress;
}

}
