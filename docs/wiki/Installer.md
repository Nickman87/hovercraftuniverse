---
title: Installer
---

# INNO Setup Script

Ik heb een INNO Setup Script gemaakt in de root van het project ("setup.iss"), zodat HovUni gemakkelijk te testen is op andere computers, zonder dat daarvoor heel het project moet worden gecheckout.

En het is ook meteen handig voor het einde van het project, aangezien we al een installer hebben. 

## Howto
  1. Build de solution in "Release" modus.
  1. Download en installeer [INNO Setup Compiler](http://www.jrsoftware.org/download.php/is.exe)
  1. Rechtsklik op _setup.iss_ en kies "Compile". De installer (_setup.exe_) zal dan in bin/installer worden gezet.

_NB_: Voor elke (nieuwe) file in de data folder die nodig is voor het programma moet er een entry staan in het INNO script.
