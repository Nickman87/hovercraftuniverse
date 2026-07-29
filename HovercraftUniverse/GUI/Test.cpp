#include "HikariDemo.h"
#include "GUIManager.h"
#include <windows.h>

int main(int argc, char *argv[]) {
	try {
		HikariDemo demo;
		while(!demo.shouldQuit) {
			demo.update();
		}
	} catch(Ogre::Exception& e) {
		ShowCursor(true);
	}

	return 0;
}