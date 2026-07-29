---
title: Config
---

Config file voorbeeld:
```
[Paths]
DataPath=..\..\data

[Ogre]
Resources=resources.cfg
Plugins=plugins_debug.cfg
LogFile=../bin/debug/HovercraftUniverse.log

[Sound]
Path=sound\
File=HovSound.fev

[Controls]
Path=controls\
File=Controls.ini

[Player]
PlayerName=UnnamedPlayer
Character=MyCharacter1
Hovercraft=SuperCraft3
```

In de client:

```
#include "Application.h"
std::string myName = Application::getConfig()->getValue("Player", "PlayerName");
```

Engine Settings CFG file:
```
[Havok]
Framerate=30
```

Gebruik in de Server-componenten:
```
#include "DedicatedServer.h"
int fps = DedicatedServer::getEngineSettings()->getIntValue("Havok", "Framerate");
```
