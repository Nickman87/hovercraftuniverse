---
title: CodeConventions
description: Iedereen moet deze regels volgen!!!
---

# Code Conventions
   
  1. Alle klassen moeten gedeclareerd worden in een header file met dezelfde naam. Dit bestand moet eindigen op .h en moet geplaatst worden in de include folder van het project in kwestie.
  1. De implementatie van een klasse moet geplaatst worden in de source file met dezelfde naam als de header file en moet eindigen op .cpp.
  1. Alles wordt gedeclareerd in een namespace. Aangezien ons project niet zo enorm groot is, gaan we geen namespaces nesten en nemen we dus gewoon als globale namespace 'HovUni'.
  1. Vermijd zoveel mogelijk includes in de header files, verkies altijd de source files.
  1. Bij het serializeren of deserializeren van data maak je bij voorkeur een subklasse van Serializer en gebruik je diens methodes.
  1. Gebruik altijd STL boven custom containers of algoritmen.
  1. Gebruik altijd C++ technieken boven C technieken (iostream vs cstdio, ...).
  1. Gebruik zo vaak mogelijk het const patroon in methoden waar nodig.
  1. Prefereer protected boven private.
  1. Declareer destructors altijd als 'virtual'.
  1. Vermijd non-const by-reference parameters.
  1. Klassen, types en structuren gebruiken altijd title case (`MyClass`).
  1. Methoden en lokale variabelen gebruiken camel case (`myMethod`).
  1. Member variables worden voorafgegaan door 'm' (`mVariable`), statische member variables worden geprefixed door 'ms' (`msStaticVar`).
  1. Preprocessor macros zijn altijd met enkel hoofdletters.
  1. Enums zijn title case en hun waarden zijn enkel hoofdletters.
  1. Klassen en methodes worden altijd volledig gedocumenteerd in het engels in de header file, gebruik makende van Doxygen-compatibele commentaar. 
  1. Gebruik nooit new delete direct, gebruik smart pointers in plaats hiervan.
  1. Zet altijd een sluitbrace '}' op een nieuwe lijn en een '{' op dezelfde lijn als het voorafgaande statement (bv. `if (imCool) {`). Uitzondering op de regel is bij lege blocks, dan doe je `if (imCool) {} `.
  1. Documenteer altijd waarom je een block leeglaat als dit zo is.
  1. Gebruik nooit volgende vorm: '`if (imCool) print "Hello"`', voeg altijd omringende braces toe, zelfs al is er maar 1 statement in de block.
  1. Gebruik typedefs indien een template-based type te ingewikkeld wordt.
  1. Voeg altijd spaces toe tussen operatoren en operands ('x + y').
  1. Haken worden altijd als volgt gebruikt: `if (imCool) {} ` en niet bijvoorbeeld `if( imCool ){} `
  1. Gebruik exception handling om fouten op te vangen, geen returnwaarden.
  1. Maak gebruik van assert().
  1. Interface classes get an "I" as prefix (ex. IResolution as interface class for Resolution)

# Code Practices

  1. Constructie en vernietiging van objecten:
    - Stel de contructie van objecten uit tot je ze echt nodig hebt. Een slecht voorbeeld: 
```
Object obj;
if (arg == 0) {
  return;
}
...
```
    - Gebruik de initializer lijst: `Vehicle(const string& name) : mName(name) { } `
    - Prefereer pre-increment (`++i`) boven post-increment (`i++`)
  1. Vermijd virtuele functie oprooepen in performantie-kritieke stukken.
  
