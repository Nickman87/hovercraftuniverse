---
title: Exceptions
description: De lijst van exceptions
---

# Exceptions

De basis exception is "Exception". Deze bevat een boodschap en kan eventueel een geneste exception bevatten.

http://fenix.cmi.ua.ac.be/~p051331/files/ExceptionInterface.png

## Nieuwe exceptions aanmaken

Als je een nieuwe exception wilt aanmaken, dan heb je 2 mogelijkheden. Ofwel maak je je eigen .h en .cpp file hiervoor, ofwel gebruik je de twee macro's uit Exception.h (HOVUNI_EXCEPTION_DECLARE en HOVUNI_EXCEPTION_DEFINITION).

Het gebruik van de 2 macro's is aangeraden wanneer de nieuwe exception geen extra functionaliteit bevat, maar eerder een andere boodschap heeft. Met deze macro's kan je dan een afgeleide klasse maken van Exception (of van een andere Exception die aangemaakt werd door deze macro's). De nieuwe exception is op deze manier een afgeleide klasse van Exception en bevat alle nodige methodes.

De macro's gebruik je zowel onderaan in Exception.h als in Exception.cpp. De macro die je gebruikt in Exception.h zorgt namelijk voor de declaratie van de klasse. De andere macro zorgt voor de definities van de methodes en bevindt zich dus in Exception.cpp.

## Exceptions throwen

Indien je exception de constructor Exception(msg, filename, linenumber) of Exception(msg, nested, filename, linenumber) implementeert, kan je gebruik maken van de THROW en THROW_NESTED macro. Deze bepaalt vanzelf de bestandsnaam en lijnnummer zodat deze gebruikt kunnen worden bij het debuggen.

## Lijst van exceptions

Hier kunnen we de lijst van exceptions, met wat uitleg, bijhouden zodat we geen dubbele zouden aanmaken.

 - NetworkException: exception als er iets mis gaat met het netwerk
