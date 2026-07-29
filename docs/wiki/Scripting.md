---
title: Scripting
---

# Content
  1. Talen
  1. Benodigde features

# Talen
Zie [CodePlea](http://codeplea.com/game-scripting-languages) voor een korte survey van de verschillende talen als embedded scripting talen, en hun huidige staat in de game development wereld.

## LUA
 - Lua
  - [Mooi voorbeeld](http://www.debian-administration.org/articles/264) 
 - LuaBind: Een C++ wrapper rond de Lua stack
  - [LuaBind documentation](http://www.rasterbar.com/products/luabind/docs.html)
  - [Quick Introduction to LuaBind](http://www.nuclex.org/articles/cxx/1-quick-introduction-to-luabind)

Voordelen:
 - Veel gebruikt binnen de game community -> veel informatie te vinden
 - Eenvoudige C++ binding
 - "kleine" maar krachtige taal
 - Performt beter dan bijvoorbeeld Python

Nadelen:
 - Controle van de stack kan complex worden (oplossing: LuaBind?)

Voorbeeld scripts:
```
function create(monster)
    monster:setRace("Sample Monster")
    monster:setDefense(10)
    monster:setAttack(10)
    monster:setLife(100)
end
```

```
---
--- Setup Script
---

load_assembly("RPGLUA"); 
NPC = import_type("RPGLUA.NPC");
Room = import_type("RPGLUA.Room");
Way = import_type("RPGLUA.PassageWay");

--Create the forest
start = CreateRoom("forest");
forest.this = start;
SetStartRoom(start);

--Create the Knight
npc = CreateNPC("knight");
knight.this = npc;

AddNPCToRoom(start, npc);

--Create the dungeon
room2 = CreateRoom("dungeon");
dungeon.this = room2;

--Create the passage from the forest to the dungeon
underground = CreatePassageWay("Steps surrounded by leaf litter", room2);
start:AddPassageWay(underground);

--Create the passage from the dungeon to the forest
overground = CreatePassageWay("a stone staircase", start);
room2:AddPassageWay(overground);
```

## Python
Ook een zeer veel gebruikte taal, is meer een gewone programmeertaal, maar kan je ondertussen zowat alles mee.
  - [Hoe te implementeren](http://docs.python.org/extending/)

Voordelen:
 - Zeer krachtig als taal, veel libraries
Nadelen:
 - Veel minder informatie over te vinden als game scripting taal
 - Lastige syntax (significant whitespace)
 - Moeilijker om te embedden (Python embedt liever zelf C++ :p)

Voorbeeld script:
```
# Tell the compiler that this code knows about generators and the keyword 'yield'
from __future__ import generators

# Define a class that will define a character's behaviour.
class MovingObject:
      # The constructor creates a handle to the generator
      # function and stores it in a private member variable.
      def __init__(self):
            self.__genHandle = self.__ UpdateFunction()
      # Update() is called once per frame and tells the update function to resume.
      def Update(self):
            return self.__genHandle.next()
      # The private object update function is resumed once per frame by Update().
      #It stores all of its state in local variables. Some of it could be stored in
      # member variables if desired.
      def __UpdateFunction(self):
            x, y = 0, 0
            dx = 0.25
            dy = 0.75
            while 1:       # Loop forever
                  # Print the current location.
                  print x, y
                  # Return the current location - in case anybody needs it.
                  yield (x, y)
                  # The function resumes here. Update the location.
                  x += dx
                  y += dy
                  # Update the velocity
                  dx += 0.1
                  dy += 0.5

# This creates an AI object that creates a generator object.
character = MovingObject()
# Run the generator object the first time - local variables
# are initialized, printed, then it yields back to here.
character.Update()
# Run the generator subsequent times. The character moves,
# prints its location, then yields, repeating as needed.
for x in range(10):
      character.Update()
```

## Andere talen
### Squirrel
 - Squirrel
  - [Officiële site](http://squirrel-lang.org/)
  - [Squirrel VS LUA](http://squirrel-lang.org/forums/thread/830.aspx)
 - SqPlus: Binding Squirrel - C++
  - [SqPlus site](http://wiki.squirrel-lang.org/default.aspx/SquirrelWiki/SqPlus.html)

Voordelen
 - Ontworpen vanuit Lua specifiek om te embedden in games
 - Bekendere syntax
Nadelen
 - Blijkt opvallend veel trager (zie de survey bovenaan deze pagina)
 - Relatief nieuw, minder gebruikt

Voorbeel script:
```
// ==========================================================
// Test.nut
// ==========================================================

function initActor( pActor ) {
   pActor.setMesh("robot.mesh");
   pActor.setScale( Vector3(3, 3, 3) );
   pActor.setPosition( Vector3( 0, 0, 0 ) );
   pActor.setMaterialName( "Examples/BumpyMetal" );
}
```

# Benodigde features
  1. Muziek dynamisch starten/stoppen (bv. afhankelijk van positie of locatie in de wereld)
  1. AI tegenstanders hun routines
