---
title: AgendaVergaderingen
description: Agendapunten voor vergaderingen
---

# Agenda voor vergaderingen

Zet hier je agendapunten voor volgende vergadering, met je naam vermeld. Op deze manier kan je je agendapunten meteen op de agenda zetten en kan iedereen voor de vergadering de agendapunten eens overlopen.

## Vergadering 03/03

 - Structuur exception (Nick)
 - Network timing issue (Olivier)
 - Entity manager (Olivier)
 - Verantwoordelijkheden (Kristof)
 - Wereld inlezen + format (PJ)
 - Entity controllers (Kristof)
 - Scripting (Dirk)

## Vergadering 10/03
 - Algemeen
  - Wereld-object (Dirk)
  - "data" map (Dirk)
  - Welke spel modus(en) gaan we maken? Time trial zoals trackmania, gewone race met x aantal laps, sprint (snelste naar finish), ...? (Nick)
 - Scripting
  - Bespreken: de manier waarop de AI aan interne gamestate variabelen kan: "Waar ben ik?", "Waar is de volgende checkpoint?", "Waar is mijn dichtstbijzijnde opponent?", enz. Alsook de vraag of het nodig is dat de AI zichzelf/anderen herkent adhv. een identifier: zo ja, waar komt deze vandaan, etc. (Dirk)
  - Komende planning en regeling scripting regelen. (Kristof)
 - Server:
  - Functionaliteit: map laden, ... (Olivier)
  - Lower level: welk soort commands (Olivier)
  - NetworkMovementEntity --> meer dynamisch object van maken zonder vaste functionaliteit. Mogelijk functies voorzien om het toevoegen van te-synchroniseren variabelen en het uitvoeren hiervan te vereenvoudigen voor de gebruiken. (Nick)

## Vergadering 17/03
 - Algemeen
  - Folder structuur
  - Hoofd project (Olivier)
 - Besluiten:
   - Folder structuur:
    - Alles in data folder dat at runtime nodig is om het spel te runnen. Zaken die niet op runtime nodig zijn mogen ergens anders staan. Geen duplicaten!
    - Tobias moet leren packages extracten.
   - Hoofd project: 
    - Exceptions: alle exceptions komen hierin
    - CoreEngine: moet in apart project komen met generieke Application klasse die overgeerfd moet worden door bv HovercraftApplication in het Hovercraft Universe project.
    - BasicEntityEvent moet bij Entity komen.
    - ControllerActionType: refactor and make generic for all types of controls (FPS, race, ...) and must be inherited by HU specific class that contains race controls for our game.
    - Bij aanmaken nieuwe entity, documenteren waar deze in de rest van de solution ook moet worden bijgevoegd voor visualizatie/networking/scripting/...
    - EntityRegister: Moet in HovercraftUniverse project komen bij networking (niet core-networking).
    - RaceCamera: generiek maken
    - GameView: indirectie verwijderen
    - EntityManager & RepresentationManager: moet pointers deleten en alles kunnen clearen.
    - CoreNetwork->Chat: in networking project
    - CoreNetwork->ClientCore/ServerCore: behouden en hernoemen naar HUClient en HUServer of dergelijke.
    - CoreNetwork->Server+ServerThread: in HU behouden.
    - CustomOgreMaxScene: in CoreEngine
    - GUI uit HU in GUI project
    - Havok (generic) in core engine en game specifieke functionaliteit in HU.
    - INI in core engine.
    - Delete TiXml
    - _Vrijdag vanaf 16u00 gaat de refactoring gebeuren tot alles in orde is._
  - *Taakverdeling:
    - Dirk: scripting met echte entiteiten
    - Olivier: helpen met server console en networking
    - PJ + Tobias: physics gebruiken op entities
    - Kristof: server console input en listentities voor staat entities; entity representations visualizeren; beginnen modelleren
    - Nick: menu *

## Vergadering 24/03
  - Algemeen
    - Hoe gaat de "GameController" gemaakt/toegevoegd worden? Gamecontroller zijnde een entiteit die verantwoordelijk is voor alle gameplay --> Starten van race, stoppen van race, bepalen wie welke positie heeft, etc. (Nick)
  - Havok
    - Havok controls / forces 
  - Scripting
    - Path definitie
    - AI Obstacle avoidance: hoe ga ik de dimensie van andere entiteiten na? Bv entiteiten met bounding box of radius, of algemeen: functie "bool probe(Vector3 position)" die direct aan Havok vraagt of er zich op dat punt een obstakel bevindt. (Dirk)
