---
title: Planning
description: The planning for the project.
---

# Lezen WPs

Aangezien ik niet per se een standaard heb gevolgd voor de planning. Hierbij enkele beslissingen:

  - Sub WP's die elkaar opvolgen (WP 1.1, WP 1.2, ...) worden verondersteld sequentieel uit te voeren en impliciet bestaat er dus een depends on binding van WP 1.2 op WP 1.1. 
  - Er zijn depends on regels die - jaja - onderlinge afhankelijkheden definieren. Het ene kan niet ontwikkeld worden als het andere niet af is.
  - Tussen WP's onderling (WP 1, WP 2) bestaat er geen dependency regels, tenzij wanneer expliciet gedefinieerd.

# Tasks

  - WP 1 (Nick + Olivier + in aan later stadium andere mensen): Ontwikkeling race state en general gameplay.
    - WP 1.1: **(DONE)** Hernoemen Player naar PlayerSettings
    - WP 1.2: **(DONE)** Ontwikkeling RaceState object dat gesynchronizeerd wordt over alle deelnemers binnen een lobby. 
    - WP 1.3: **(DONE)** Implementatie RacePlayer object met PlayerSettings (ontvangen door Lobby) en Positie. 
    - WP 1.4: **(DONE)** RaceState creeert voor alle deelnemers in de Lobby een RacePlayer object, wat wordt gesynchronizeerd over alle deelnemers. 
    - WP 1.5: **(DONE)** Visualizatie alle deelnemers in plaats van enkel diegene die controle heeft over het keyboard. Hierbij hoort het inlezen van de starting posities van de wereldbeschrijving en het plaatsen van de hovercrafts op hun posities (depends on 5.3.3 voor minstens 1 map).
    - WP 1.6: **(DONE)** Implementatie staten binnen de race state (count down, racing, finished, ...). 
    - WP 1.7: **(DONE)** Events triggeren naar clients als gevolg van de staten (tonen nummers bij count down, tonen ranking bij finish, ...).
    - WP 1.8: **(DONE)** Player specific events (voorbijsteken, bereiken finish, bereiken checkpoint, ...) triggeren en koppelen aan RacePlayer, deze events ook visualizeren bij entities (juichen, ...).
    - WP 1.9: **(IGNORE)** Uitbreiden speler met emotional state en agressiveness behaviour.
    - WP 1.10: **(DONE)** Uitbreiden racestate met informatie over de checkpoints en over welke players welke checkpoints reeds hebben bereikt (depends on WP 2.3).
    - WP 1.11.A: Toelaten dat er een unload plaatsvindt zodat entiteiten uit de client verwijderd kunnen worden.
    - WP 1.11.B: **(DONE)** Uitbreiden lobby zodat na het eindigen van een race de lobby opnieuw geopend wordt (en dat bij het starten van een race nadien de posities van het eindigen van een spel voordien behouden blijft).
   - WP 1.12: Implementatie reset functie (bij drukken op reset knop of bij vallen in put). Deze brengt je terug onder laatst bereikte checkpoint. 

  - WP 2 (Tobias + PJ): **(DONE)** Integratie physics voor alle types entiteiten.
    - WP 2.1: **(Done)** Implementatie generieke physics voor wereld (zwaartekracht, botsing, ...)
    - WP 2.2: **(Done)** Implementatie physics voor custom entiteiten (depends on WP 4).
     - WP 2.2.1: **(Done)** Implementatie physics boost a.d.h.v. strength/acceleratie.
     - WP 2.2.2: **(Done)** Implementatie physics jump a.d.h.v. up force.
    - WP 2.3: **(DONE)** Toelaten dat Havok events kan sturen ivm checkpoints naar de race state.

  - WP 4 (Kristof): **(DONE)** Toevoegen eigenschappen aan entiteiten.
    - WP 4.1: **(Done)** Geef weight/max speed/acceleration/steering parameters aan hovercraft.
    - WP 4.2: **(Done)** Geef eigenschappen aan planeet (gravity, ...).

  - WP 5 (Kristof + PJ): 3D art development + sound
    - WP 5.1: Design hovercrafts.
     - WP 5.1.1: Design meerdere hovercrafts (modeling).
     - WP 5.1.2: Mapping materials op hovercrafts.
     - WP 5.1.3: Toekenning eigenschappen aan hovercrafts (depends on WP 4.1).
    - WP 5.2: Design models.
     - WP 5.2.1: Modeling models.
     - WP 5.2.2: Materialen mappen op models.
     - WP 5.2.3: Toekennen animaties aan models.
     - WP 5.2.4: Koppelen animaties aan events (depends on WP 1.8).
    - WP 5.3: Design single asteroids/planeten.
     - WP 5.3.1: Modeling planets.
     - WP 5.3.2: Materialen mappen op planets.
     - WP 5.3.3: Planets compliant maken zodat ze ingelezen kunnen worden door engine.
     - WP 5.3.4: Koppelen eigenschappen planeet aan mesh.
    - WP 5.4: Design maps (depends on WP1.10, WP2).
      - WP 5.4.1: Implementatie pauze functie die huidige positie in x, y, z weergeeft zodat plaatsing nieuwe planeten waarnaar gejumpt moet worden gemakkelijker kan.
      - WP 5.4.2: Plaatsing planeten om een hele map te verkrijgen.
      - WP 5.4.3: Toelaten dat race state map laadt.
    - WP 5.5: Toekennen sounds aan events.
   
   - WP 6 (Dirk): artificial intelligence
    - WP 6.1: **(DONE)** Implementatie path finding algoritmen in generieke context via AIControllers.
    - WP 6.2: **(DONE)** Volgen path gedefinieerd door map path (depends on WP 5.3.4 voor minstens 1 map).
    - WP 6.3: **(DONE)** Implementatie aanwezigheid bots in server met meerdere characters (depends on  WP 1.5).
    - WP 6.4: **(DONE)** Ontvangen Havok events die potentiele collisions aangeeft, waardoor collision avoidance kan uitgevoerd worden.
    - WP 6.5: Bijkomend op collision avoidance, implementatie path planning die meer op voorhand gaat plannen dan voor collision avoidance (optioneel).
    - WP 6.6: Gebruik maken van kennis entiteiten (boost, jump, ...) om nog optimalere resultaten te behalen (optioneel) (depends on WP 5.3.4 voor minstens 1 map).
    - WP 6.7: Agressiveness of emotional state gebruiken om AI meer random en spelafhankelijk gedrag te geven (optioneel) (depends on WP 1.9).

   - WP 7 (Kristof + PJ): Rendering
     - WP 7.1: **(DONE)** Implementation shaders.
     - WP 7.2: **(DONE)** General improvement in graphics engine when needed.
     - WP 7.3: **(DONE)** Statische objecten introduceren om tegen te rijden.
     - WP 7.4: **(IGNORE)** Implementatie view ports die character al racend tonen (optioneel).

   - WP 8 (Nick): Menu + HUD
    - WP 8.1: **(IGNORE)** Visualizatie control mapping + bieden mogelijkheden om controls aan te passen en op te slaan voor volgende keer.
    - WP 8.2: Visualizatie pijl zodat die de path volgt (of volgende checkpoint) om zo de richting waarin een Hovercraft zou moeten bewegen aan te duiden (depends on WP 6.1 indien path following).
    - WP 8.3: **(DONE)** Visualizatie tijd racing.
    - WP 8.4: **(DONE)** Update speedometer + boost (TODO: wat doen we met boost?).
    - WP 8.5: Visualizatie achterstand op eerste + achterstand op vorige racer bij passeren checkpoint (depends on WP 1.10).
    - WP 8.6: **(IGNORE)** Visualizatie rankings voorgaande en volgende spelers als live 3D model (depends on WP 7.4, WP 1.7).

   - WP 10 (iedereen): Finalizatie
    - WP 10.1: Code review 
    - WP 10.2: Gameplay testing (depends on 1.10).
    - WP 10.3: Verpakking + installer.
    - WP 10.4: Schrijven eindverslag.

# Gantt chart

http://www.delahaye.it/uploader/files/gantt.png
 
