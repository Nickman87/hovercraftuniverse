#include "HUApplication.h"
#include "HUDedicatedServer.h"
#include "HUServer.h"
#include "EntityManager.h"
#include "Console.h"
#include <OgreString.h>
#include "Exception.h"
// Modern-build fix (docs/porting/hikari-gui.md): ZoidCom's declaration is
// needed directly here (previously only ever compiled transitively via
// some other header in the VC9 project's precompiled-header chain).
#include <zoidcom/zoidcom.h>

void process_zoidcom_log(const char *_log) {
	Ogre::LogManager::getSingleton().getDefaultLog()->stream() << _log;
}

/**
 * Hovercraft Universe Application entry point.
 *
 * @author Kristof Overdulve & Olivier Berghmans & Pieter-Jan Pintens
 */
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT) {
	// Create ZoidCom
	ZoidCom *zcom = new ZoidCom(process_zoidcom_log);
	if (!zcom || !zcom->Init()) {
		return 1;
	}

	bool console = false;
	bool server = false;
	unsigned int port = 2375;
	Ogre::String host = "localhost";
	//parse all commandline parameters (seperated by spaces)
	Ogre::String commandline (strCmdLine);
	// Ogre 14 API fix (docs/porting/ogre-api-gap.md, row 1): the
	// Ogre::vector<T>::type STLAllocator-wrapper idiom is gone; modern Ogre
	// (and StringUtil::split's return type) just uses std::vector<T>.
	std::vector<Ogre::String> result = Ogre::StringUtil::split(commandline, " ");
	for (std::vector<Ogre::String>::iterator i = result.begin(); i != result.end(); i++ ) {
		if ((*i) == "--server") {
			server = true;
		} else if (Ogre::StringUtil::startsWith(*i,"--host=")) {
			//string of the form host:port
			Ogre::String connectionstring = (*i).substr(7);
			size_t pos = connectionstring.find(":");
			if (pos == Ogre::String::npos) {
				host = connectionstring;
			} else {
				host = connectionstring.substr(0,pos);
				port = Ogre::StringConverter::parseInt(connectionstring.substr(pos+1));
			}
		} else if ((*i) == "--console") {
			console = true;
		}
	}

	if (server) {
		HovUni::Console::createConsole("HovercraftUniverse Dedicated Server");
		HovUni::HUDedicatedServer app("Server.ini");
		try {
			app.init();
			app.run();
		} catch (Ogre::Exception& e) {
			MessageBox(NULL, e.getFullDescription().c_str(), "An Ogre exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		} catch (HovUni::Exception& e2) {
			MessageBox(NULL, e2.getMessage().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		} catch (std::exception& e) {
			MessageBox(NULL, e.what(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);		
		} catch (...) {
			MessageBox(NULL, "Unknown fatal exception!", "An error has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		}
		HovUni::Console::destroyConsole();

	} else {
		if (console) {
			HovUni::Console::createConsole("HovercraftUniverse Debug Console");
		}

		HovUni::HUApplication app("HovercraftUniverse.ini");
		
		try {
			app.init();
			app.go(host,port);
		} catch (Ogre::Exception & e) {
			MessageBox(NULL, e.getFullDescription().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		} catch (HovUni::Exception e2) {
			MessageBox(NULL, e2.getMessage().c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		} catch (std::exception e) {
			MessageBox(NULL, e.what(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		} catch (...) {
			MessageBox(NULL, "Unknown fatal exception!", "An error has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
		}

		if ( console )
			HovUni::Console::destroyConsole();
	}

	delete zcom;
	return 0;
}