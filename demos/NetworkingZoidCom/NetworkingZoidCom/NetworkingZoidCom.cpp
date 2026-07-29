// NetworkingZoidCom.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <conio.h>
#include <zoidcom/zoidcom.h>
#include "SampleServer.h"
#include "SampleClient.h"
#include "Tree.h"
#include "Player.h"

void process_zoidcom_log(const char *_log) {
	printf("%s\n", _log);
}

void makeServer(ZoidCom* zcom) {
	// make instance of our Server class
	SampleServer *server = new SampleServer();

	bool exit = false;
	while (!exit) {
		if (_kbhit()) {
			char c = _getch();
			switch (c) {
				case 'q':
				case 'Q':
					exit = true;
					break;
				case 't': 
					{
						Tree* tree = new Tree(server);
						server->addEntity(tree);
						break;
					}
				default:
					break;
			}
		}

		// let the server do processing
		server->process();
		// let the program sleep for 0 msecs
		zcom->Sleep(1);
	}
	delete server;
}

void makeClient(ZoidCom* zcom) {
	// make instance of our Client class
	SampleClient *client = new SampleClient("localhost");

	bool exit = false;
	while (!exit && !client->canExit()) {
		if (_kbhit()) {
			char c = _getch();
			switch (c) {
				case 'q':
				case 'Q':
					exit = true;
					break;
				case 'w': 
					client->moveForward();
					break;
				case 's': 
					client->moveBackward();
					break;
				case 'a': 
					client->moveLeft();
					break;
				case 'd': 
					client->moveRight();
					break;
				default:
					break;
			}
		}

		// let the client do processing
		client->process();
		// give up remaining cpu time
		zcom->Sleep(1);
	}

	delete client;
}


void makeBoth(ZoidCom* zcom) {
	// make instance of our Server class
	SampleServer *server = new SampleServer();
	// make instance of our Client class
	SampleClient *client = new SampleClient();

	bool exit = false;
	while (!exit && !client->canExit()) {
		if (_kbhit()) {
			char c = _getch();
			switch (c) {
				case 'q':
				case 'Q':
					exit = true;
					break;
				case 't': 
					{
						Tree* tree = new Tree(server);
						server->addEntity(tree);
						break;
					}
				case 'w': 
					client->moveForward();
					break;
				case 's': 
					client->moveBackward();
					break;
				case 'a': 
					client->moveLeft();
					break;
				case 'd': 
					client->moveRight();
					break;
				default:
					break;
			}
		}

		// let the server do processing
		server->process();
		// let the client do processing
		client->process();
		// let the program sleep for 0 msecs
		zcom->Sleep(1);
	}

	delete server;
	delete client;
}

int _tmain(int argc, _TCHAR* argv[])
{
	char ch;
	cout << "Start as (c)lient, (s)erver or (b)oth? " << endl;
	std::cin >> ch;

	bool server = true;
	bool client = false;
	switch (ch) {
	  case 'c':
	  case 'C':
		  cout << "Client" << endl;
		  server = false;
		  client = true;
		  break;
	  case 's':
	  case 'S':
		  cout << "Server" << endl;
		  server = true;
		  client = false;
		  break;
	  case 'b':
	  case 'B':
		  cout << "Both" << endl;
		  server = true;
		  client = true;
		  break;
	  default:
		  return 1;
	}

	// create ZoidCom with logging disabled
	//ZoidCom *zcom = new ZoidCom(process_zoidcom_log); // Console logging
	ZoidCom *zcom = new ZoidCom(); // No extra console logging
	if (!zcom || !zcom->Init()) {
		return 1;
	}

	// here Zoidcom can be used
	if (server && client) {
		makeBoth(zcom);
	} else if (server) {
		makeServer(zcom);
	} else {
		makeClient(zcom);
	}

	// remove it again
	delete zcom;
	return 0;
}

