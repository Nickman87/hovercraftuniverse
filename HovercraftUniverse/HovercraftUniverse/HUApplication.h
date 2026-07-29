#ifndef HUAPPLICATION_H_
#define HUAPPLICATION_H_

#include "Application.h"
#include "ClientPreparationLoader.h"
#include "ProgressMonitorListener.h"
#include <string>

namespace HovUni {

	/**
	* Main application for the Hovercraft Universe game.
	*
	* @author Kristof Overdulve, Dirk Delahaye
	*/
	class HUApplication : public Application, public IProgressMonitorListener {
	public:

		/** The client preparation loader that can be used to read and reread scenes to render */
		static ClientPreparationLoader * msPreparationLoader;

		/**
		* Constructor.
		*/
		HUApplication(const std::string& configINI);

		/**
		* Destructor.
		*/
		~HUApplication();

		/**
		 * @see Application::playMusic().
		 */
		void playMusic(SoundManager * soundMgr);

		/**
		 * @see Application::getInitialGameState().
		 */
		BasicGameState * getInitialGameState();

		/**
		 * @see Application::customSceneSetup().
		 */
		void customSceneSetup();

		/**
		 * @see Application::init().
		 */
		void init();

		/**
		 * @see IProgressMonitorListener::updateProgress().
		 */
		void updateProgress(double progress);
	};

#endif

}