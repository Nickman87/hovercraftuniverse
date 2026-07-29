---
title: CompileAndRun
---

# Introduction

Compilatie & Run.

# Details

SVN checkout:

http://code.google.com/p/uhasseltaacgua/source/checkout

Het project staat in `"trunk\HovercraftUniverse"`, vanaf nu ook wel de project map genoemd.

DEPENDENCIES: windows packages

  1. Download alle windows packages van onze download pagina:

  http://code.google.com/p/uhasseltaacgua/downloads/list

  1. Extract deze allemaal in de project map, merge wanneer gevraagd...

COMPILE

  1. Open `trunk\HovercraftUniverse\HovercraftUniverse.sln`
  1. Duw op F7

RUN

  1. Rechts klik op HovercraftUniverse project (eerste subitem in de boom links).
  1. Kies properties uit de drop down menu, nu opent een nieuw venster.
  1. Kies Debugging en vul Working directory in afhankelijk van de mode, deze lees je naast Configuration in de titelbar en is of Debug of Release.
    - Voor Debug mode hebt vul dit in: `$(SolutionDir)bin\debug`
    - Voor Release mode hebt vul dit in: `$(SolutionDir)bin\release`
  1. Kies toepassen
  1. Als je Windows XP hebt moet directx10 renderer uitschakelen. Dit doe je door in de `bin\debug` en `bin\release` mappen in de project map de file plugins.cfg aan te passen. Zet een `#` voor de directx10 renderer.
  1. Duw op F5

EXTRA

Zet volgende regels onder `Zip=../../media/packs/skybox.zip`

`FileSystem=../../../art/materials`

`FileSystem=../../../art/materials/scripts`

`FileSystem=../../../art/materials/textures`

`FileSystem=../../../art/models`

In `$(SolutionDir)bin\debug\resource.cfg` en `$(SolutionDir)bin\release\resource.cfg` om onze art te gebruiken!
